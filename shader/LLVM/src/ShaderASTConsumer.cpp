#include "DebugASTVisitor.hpp"
#include "ShaderASTConsumer.hpp"
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>
#include <clang/AST/DeclTemplate.h>
#include <format>

namespace skr::SSL {

inline void ASTConsumer::ReportFatalError(const std::string& message)
{
    llvm::report_fatal_error(message.c_str());
}

template <typename... Args>
inline void ASTConsumer::ReportFatalError(std::format_string<Args...> _fmt, Args&&... args)
{
    auto message = std::format(_fmt, std::forward<Args>(args)...);
    llvm::report_fatal_error(message.c_str());
}

template <typename... Args>
inline void ASTConsumer::ReportFatalError(const clang::Stmt* expr, std::format_string<Args...> _fmt, Args&&... args)
{
    DumpWithLocation(expr);
    ReportFatalError(_fmt, std::forward<Args>(args)...);
}

void ASTConsumer::DumpWithLocation(const clang::Stmt *stmt)
{
    stmt->getBeginLoc().dump(pASTContext->getSourceManager());
    stmt->dump();
}

inline static String ToText(clang::StringRef str)
{
    return String(str.begin(), str.end());
}

template <typename T>
inline static T GetArgumentAt(const clang::AnnotateAttr* attr, size_t index)
{
    auto args = attr->args_begin() + index;
    if constexpr (std::is_same_v<T, clang::StringRef>)
    {
        auto arg = llvm::dyn_cast<clang::StringLiteral>((*args)->IgnoreParenCasts());
        return arg->getString();
    }
    else
    {
        static_assert(std::is_same_v<T, std::nullptr_t>, "Unsupported type for GetArgumentAt");
    }
}

inline static clang::AnnotateAttr* ExistShaderAttrWithName(const clang::Decl* decl, const char* name)
{
    auto attrs = decl->specific_attrs<clang::AnnotateAttr>();
    for (auto attr : attrs)
    {
        if (attr->getAnnotation() != "skr-shader")
            continue;
        if (GetArgumentAt<clang::StringRef>(attr, 0) == name)
            return attr;
    }
    return nullptr;
}

inline static clang::AnnotateAttr* IsIgnore(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "ignore"); }
inline static clang::AnnotateAttr* IsBuiltin(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "builtin"); }
inline static clang::AnnotateAttr* IsDump(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "dump"); }
inline static clang::AnnotateAttr* IsKernel(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "kernel"); }

CompileFrontendAction::CompileFrontendAction(skr::SSL::AST& AST)
    : clang::ASTFrontendAction(), AST(AST)
{
}

std::unique_ptr<clang::ASTConsumer> CompileFrontendAction::CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile)
{
    auto &LO = CI.getLangOpts();
    LO.CommentOpts.ParseAllComments = true;
    return std::make_unique<skr::SSL::ASTConsumer>(AST);
}

ASTConsumer::ASTConsumer(skr::SSL::AST& AST)
    : clang::ASTConsumer(), AST(AST)
{

}

ASTConsumer::~ASTConsumer()
{
    
}

// clang::DeclRefExpr* cap;
// cap->refersToEnclosingVariableOrCapture();

void ASTConsumer::HandleTranslationUnit(clang::ASTContext& Context)
{
    pASTContext = &Context;
    
    DebugASTVisitor debug = {};
    debug.TraverseDecl(Context.getTranslationUnitDecl());

    // add primitive type mappings
    addType(Context.VoidTy->getTypePtr(), AST.VoidType);
    addType(Context.BoolTy->getTypePtr(), AST.BoolType);
    addType(Context.FloatTy->getTypePtr(), AST.FloatType);
    addType(Context.UnsignedIntTy->getTypePtr(), AST.UIntType);
    addType(Context.IntTy->getTypePtr(), AST.IntType);
    addType(Context.DoubleTy->getTypePtr(), AST.DoubleType);
    addType(Context.UnsignedLongLongTy->getTypePtr(), AST.U64Type);
    addType(Context.LongLongTy->getTypePtr(), AST.I64Type);

    // add record types
    TraverseDecl(Context.getTranslationUnitDecl());
}

bool ASTConsumer::VisitEnumDecl(clang::EnumDecl* enumDecl)
{
    using namespace clang;

    if (IsDump(enumDecl))
        enumDecl->dump();

    auto UnderlyingType = getType(enumDecl->getIntegerType().getTypePtr());
    addType(enumDecl->getTypeForDecl(), UnderlyingType);

    auto EnumName = enumDecl->getQualifiedNameAsString();
    std::replace(EnumName.begin(), EnumName.end(), ':', '_');
    for (auto E : enumDecl->enumerators())
    {
        const auto I = E->getInitVal().getLimitedValue();
        auto VarName = (EnumName + "__" + E->getName()).str();
        AST.DeclareGlobalConstant(UnderlyingType, ToText(VarName), AST.Constant(IntValue(I)));
    }

    return true;
}

bool ASTConsumer::VisitRecordDecl(clang::RecordDecl* recordDecl)
{    
    using namespace clang;

    if (IsDump(recordDecl))
        recordDecl->dump();

    const auto* Type = recordDecl->getTypeForDecl();
    const auto* TSD = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(recordDecl);
    const auto* TSD_Partial = llvm::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(recordDecl);
    const auto* TemplateItSelf = recordDecl->getDescribedTemplate();
    if (recordDecl->isUnion()) return true; // unions are not supported
    if (!recordDecl->isCompleteDefinition()) return true; // skip forward declares
    if (TSD && !TSD->isCompleteDefinition()) return true; // skip no-def template specs
    if (TSD && TSD_Partial) return true; // skip no-def template specs
    if (!TSD && TemplateItSelf) return true; // skip template definitions
    if (IsIgnore(recordDecl)) return true; // skip ignored types

    if (auto BuiltinAttr = IsBuiltin(recordDecl))
    {
        auto What = GetArgumentAt<clang::StringRef>(BuiltinAttr, 1);
        if (What == "vec")
        {
            const auto& Arguments = TSD->getTemplateArgs();
            const auto* ET = Arguments.get(0).getAsType().getCanonicalType()->getAs<clang::BuiltinType>();
            const uint64_t N = Arguments.get(1).getAsIntegral().getLimitedValue();
            
            if (N <= 1 || N > 4) 
                ReportFatalError("Unsupported vec size: " + std::to_string(N));

            if (getType(ET) == AST.FloatType)
            {
                skr::SSL::TypeDecl* Types[] = { AST.Float2Type, AST.Float3Type, AST.Float4Type };
                addType(Type, Types[N - 2]);
            }
            else if (getType(ET) == AST.IntType)
            {
                skr::SSL::TypeDecl* Types[] = { AST.Int2Type, AST.Int3Type, AST.Int4Type };
                addType(Type, Types[N - 2]);
            }
            else if (getType(ET) == AST.UIntType)
            {
                skr::SSL::TypeDecl* Types[] = { AST.UInt2Type, AST.UInt3Type, AST.UInt4Type };
                addType(Type, Types[N - 2]);
            }
            else if (getType(ET) == AST.BoolType)
            {
                skr::SSL::TypeDecl* Types[] = { AST.Bool2Type, AST.Bool3Type, AST.Bool4Type };
                addType(Type, Types[N - 2]);
            }
            else
            {
                ReportFatalError("Unsupported vec type: " + std::string(ET->getTypeClassName()) + " for vec size: " + std::to_string(N));
                recordDecl->dump();
            }
        }
        else if (What == "array")
        {
            const auto& Arguments = TSD->getTemplateArgs();
            const auto* ET = Arguments.get(0).getAsType().getTypePtr();
            const auto N = Arguments.get(1).getAsIntegral().getLimitedValue();
            auto ArrayType = AST.DeclareArrayType(getType(ET), uint32_t(N));
            addType(Type, ArrayType);
        }
        else if (What == "matrix")
        {
            const auto& Arguments = TSD->getTemplateArgs();
            const auto N = Arguments.get(0).getAsIntegral().getLimitedValue();
            skr::SSL::TypeDecl* Types[] = { AST.Float2x2Type, AST.Float3x3Type, AST.Float4x4Type };
            addType(Type, Types[N - 2]);
        }
        return true;
    } 

    if (getType(Type))
        ReportFatalError("Duplicate type declaration: " + std::string(recordDecl->getName()));

    auto NewType = AST.DeclareType(ToText(recordDecl->getName()), {});
    for (auto field : recordDecl->fields())
    {
        if (IsDump(field)) 
            field->dump();

        auto desugaredFTy = field->getType().getCanonicalType();
        auto fieldType = desugaredFTy->getAs<clang::Type>();

        if (auto ft = getType(fieldType))
            NewType->add_field(AST.DeclareField(ToText(field->getName()), ft));
        else
            ReportFatalError("Unknown field type: " + std::string(fieldType->getTypeClassName()) + " for field: " + field->getName().str());
    } 
    addType(Type, NewType);
    return true;
}

bool ASTConsumer::VisitFunctionDecl(clang::FunctionDecl* x)
{
    if (IsKernel(x))
    {
        auto CxxFunctionName = x->getQualifiedNameAsString();
        std::replace(CxxFunctionName.begin(), CxxFunctionName.end(), ':', '_');
        AST.DeclareFunction(ToText(CxxFunctionName),
            getType(x->getReturnType().getTypePtr()),
            {},
            traverseStmt<SSL::CompoundStmt>(x->getBody())
        );
    }
    return true;
}

template <typename T>
T* ASTConsumer::traverseStmt(const clang::Stmt *x) 
{
    return (T*)traverseStmt(x);
}

Stmt* ASTConsumer::traverseStmt(const clang::Stmt *x) 
{
    using namespace clang;
    using namespace skr;

    if (x == nullptr) 
        return nullptr;

    if (auto cxxBranch = llvm::dyn_cast<clang::IfStmt>(x)) 
    {
        auto cxxCond = cxxBranch->getCond();
        auto ifConstVar = cxxCond->getIntegerConstantExpr(*pASTContext);
        if (ifConstVar) {
            if (ifConstVar->getExtValue() != 0) {
                if (cxxBranch->getThen())
                    traverseStmt(cxxBranch->getThen());
            } else {
                if (cxxBranch->getElse())
                    traverseStmt(cxxBranch->getElse());
            }
        } else {
            auto _cond = traverseStmt<SSL::Expr>(cxxCond);
            auto _then = traverseStmt<SSL::CompoundStmt>(cxxBranch->getThen());
            auto _else = traverseStmt<SSL::CompoundStmt>(cxxBranch->getElse());
            return AST.If(_cond, _then, _else);
        }
    } 
    else if (auto cxxSwitch = llvm::dyn_cast<clang::SwitchStmt>(x)) 
    {
        std::vector<SSL::CaseStmt*> cases;
        std::vector<const clang::SwitchCase*> cxxCases;
        if (auto caseList = cxxSwitch->getSwitchCaseList()) 
        {
            while (caseList) {
                cxxCases.emplace_back(caseList);
                caseList = caseList->getNextSwitchCase();
            }
            std::reverse(cxxCases.begin(), cxxCases.end());
            cases.reserve(cxxCases.size());
            for (auto cxxCase : cxxCases)
                cases.emplace_back(traverseStmt<SSL::CaseStmt>(cxxCase));
        }
        return AST.Switch(traverseStmt<SSL::Expr>(cxxSwitch->getCond()), cases);
    } 
    else if (auto cxxCase = llvm::dyn_cast<clang::CaseStmt>(x)) 
    {
        return AST.Case(traverseStmt<SSL::Expr>(cxxCase->getLHS()), traverseStmt<SSL::CompoundStmt>(cxxCase->getSubStmt()));
    } 
    else if (auto cxxDefault = llvm::dyn_cast<clang::DefaultStmt>(x)) 
    {
        auto _body = traverseStmt<SSL::CompoundStmt>(cxxDefault->getSubStmt());
        return AST.Default(_body);
    } 
    else if (auto cxxContinue = llvm::dyn_cast<clang::ContinueStmt>(x)) 
    {
        return AST.Continue();
    } 
    else if (auto cxxBreak = llvm::dyn_cast<clang::BreakStmt>(x)) 
    {
        return AST.Break();
    } 
    else if (auto cxxWhile = llvm::dyn_cast<clang::WhileStmt>(x)) 
    {
        auto _cond = traverseStmt<SSL::Expr>(cxxWhile->getCond());
        return AST.While(_cond, traverseStmt<SSL::CompoundStmt>(cxxWhile->getBody()));
    } 
    else if (auto cxxFor = llvm::dyn_cast<clang::ForStmt>(x)) 
    {
        auto _init = traverseStmt(cxxFor->getInit());
        auto _cond = traverseStmt<SSL::Expr>(cxxFor->getCond());
        auto _inc = traverseStmt(cxxFor->getInc());
        auto _body = traverseStmt<SSL::CompoundStmt>(cxxFor->getBody());
        return AST.For(_init, _cond, _inc, _body);
    } 
    else if (auto cxxCompound = llvm::dyn_cast<clang::CompoundStmt>(x)) 
    {
        std::vector<SSL::Stmt*> stmts;
        stmts.reserve(cxxCompound->size());
        for (auto sub : cxxCompound->body())
            stmts.emplace_back(traverseStmt(sub));
        return AST.Block(std::move(stmts));
    }
    else if (auto cxxExprWithCleanup = llvm::dyn_cast<clang::ExprWithCleanups>(x))
    {
        return traverseStmt(cxxExprWithCleanup->getSubExpr());
    }
    ///////////////////////////////////// EXPRS ///////////////////////////////////////////
    else if (auto cxxLambda = llvm::dyn_cast<LambdaExpr>(x)) 
    {
        ReportFatalError("Lambda expressions are not supported in skr-shader.");
        return nullptr;
    }
    else if (auto cxxDecl = llvm::dyn_cast<clang::DeclStmt>(x)) 
    {
        const DeclGroupRef declGroup = cxxDecl->getDeclGroup();
        for (auto decl : declGroup) {
            if (!decl) continue;

            if (auto *varDecl = dyn_cast<clang::VarDecl>(decl)) {
                auto Ty = varDecl->getType();
                auto cxxInit = varDecl->getInit();
                const bool isConst = varDecl->getType().isConstQualified();
                const bool isRef = Ty->isReferenceType();
                const bool isArray = Ty->getAsArrayTypeUnsafe();
                if (isRef || isArray) {
                    if (isRef)
                        ReportFatalError(x, "VarDecl as reference type is not supported: [{}]", Ty.getAsString());
                    if (isArray)
                        ReportFatalError(x, "VarDecl as C-style array type is not supported: [{}]", Ty.getAsString());
                }
                if (auto SSLType = getType(Ty.getCanonicalType().getTypePtr()))
                {
                    auto _init = traverseStmt<SSL::Expr>(cxxInit);
                    auto _name = SSL::String(varDecl->getName().begin(), varDecl->getName().end());
                    AST.Variable(isConst ? SSL::EVariableQualifier::Const : SSL::EVariableQualifier::None, SSLType, _name, _init);
                } 
                else
                    ReportFatalError("VarDecl with unfound type: [{}]", Ty.getAsString());
            } else if (auto aliasDecl = dyn_cast<clang::TypeAliasDecl>(decl)) {          // ignore
            } else if (auto staticAssertDecl = dyn_cast<clang::StaticAssertDecl>(decl)) {// ignore
            } else {
                ReportFatalError(x, "unsupported decl stmt: {}", cxxDecl->getStmtClassName());
            }
        }
    }
    else if (auto cxxImplicitCast = llvm::dyn_cast<clang::ImplicitCastExpr>(x))
    {
        auto SSLType = getType(cxxImplicitCast->getType().getTypePtr());
        return AST.ImplicitCast(SSLType, traverseStmt<SSL::Expr>(cxxImplicitCast->getSubExpr()));
    }
    else if (auto cxxTempObject = llvm::dyn_cast<clang::CXXConstructExpr>(x))
    {

    }

    ReportFatalError(x, "unsupported stmt: {}", x->getStmtClassName());
    return nullptr;
}

bool ASTConsumer::addType(const clang::Type* type, skr::SSL::TypeDecl* decl)
{
    if (auto bt = type->getAs<clang::BuiltinType>())
    {
        auto kind = bt->getKind();
        if (_builtin_types.find(kind) != _builtin_types.end())
        {
            ReportFatalError("Duplicate builtin type declaration: " + std::string(bt->getTypeClassName()));
            return false;
        }
        _builtin_types[kind] = decl;
    }
    else if (auto tag = type->getAsTagDecl())
    {
        if (_tag_types.find(tag) != _tag_types.end())
        {
            ReportFatalError("Duplicate tag type declaration: " + std::string(tag->getName()));
            return false;
        }
        _tag_types[tag] = decl;
    }
    else
    {
        ReportFatalError("Unknown type declaration: " + std::string(type->getTypeClassName()));
        return false;
    }
    return true;
}

skr::SSL::TypeDecl* ASTConsumer::getType(const clang::Type* type) const
{
    if (auto bt = type->getAs<clang::BuiltinType>())
    {
        auto kind = bt->getKind();
        if (_builtin_types.find(kind) != _builtin_types.end())
            return _builtin_types.at(kind);
    }
    else if (auto tag = type->getAsTagDecl())
    {
        if (_tag_types.find(tag) != _tag_types.end())
            return _tag_types.at(tag);
    }
    return nullptr;
}

} // namespace skr::SSL