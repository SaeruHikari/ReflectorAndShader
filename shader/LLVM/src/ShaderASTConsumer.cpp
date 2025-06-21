#include "DebugASTVisitor.hpp"
#include "ShaderASTConsumer.hpp"
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>
#include <clang/AST/DeclTemplate.h>
#include <format>

namespace skr::SSL {

const bool LanguageRule_UseAssignForImplicitCopyOrMove(const clang::Decl* x)
{
    if (auto AsMethod = llvm::dyn_cast<clang::CXXMethodDecl>(x))
    {
        const bool isImplicit = x->isImplicit();
        bool isCopyOrMove = AsMethod->isCopyAssignmentOperator() || AsMethod->isMoveAssignmentOperator();
        if (auto AsCtor = llvm::dyn_cast<clang::CXXConstructorDecl>(x))
        {
            isCopyOrMove = AsCtor->isCopyConstructor() || AsCtor->isMoveConstructor();
        }
        if (isImplicit && isCopyOrMove)
            return true;
    }
    return false;
}

inline void ASTConsumer::ReportFatalError(const std::string& message) const
{
    llvm::report_fatal_error(message.c_str());
}

template <typename... Args>
inline void ASTConsumer::ReportFatalError(std::format_string<Args...> _fmt, Args&&... args) const
{
    auto message = std::format(_fmt, std::forward<Args>(args)...);
    llvm::report_fatal_error(message.c_str());
}

template <typename... Args>
inline void ASTConsumer::ReportFatalError(const clang::Stmt* expr, std::format_string<Args...> _fmt, Args&&... args) const
{
    DumpWithLocation(expr);
    ReportFatalError(_fmt, std::forward<Args>(args)...);
}

void ASTConsumer::DumpWithLocation(const clang::Stmt *stmt) const
{
    stmt->getBeginLoc().dump(pASTContext->getSourceManager());
    stmt->dump();
}

inline static String ToText(clang::StringRef str)
{
    return String(str.begin(), str.end());
}

inline static SSL::UnaryOp TranslateUnaryOp(clang::UnaryOperatorKind op)
{
    switch (op)
    {
        case clang::UO_Plus: return SSL::UnaryOp::PLUS;
        case clang::UO_Minus: return SSL::UnaryOp::MINUS;
        case clang::UO_LNot: return SSL::UnaryOp::NOT;
        case clang::UO_Not: return SSL::UnaryOp::BIT_NOT;

        case clang::UO_PreInc: return SSL::UnaryOp::PRE_INC;
        case clang::UO_PreDec: return SSL::UnaryOp::PRE_DEC;
        case clang::UO_PostInc: return SSL::UnaryOp::POST_INC;
        case clang::UO_PostDec: return SSL::UnaryOp::POST_DEC;
        default:
            llvm::report_fatal_error("Unsupported unary operator");
    }
}

inline static SSL::BinaryOp TranslateBinaryOp(clang::BinaryOperatorKind op)
{
    switch (op)
    {
        case clang::BO_Add: return SSL::BinaryOp::ADD;
        case clang::BO_Sub: return SSL::BinaryOp::SUB;
        case clang::BO_Mul: return SSL::BinaryOp::MUL;
        case clang::BO_Div: return SSL::BinaryOp::DIV;
        case clang::BO_Rem: return SSL::BinaryOp::MOD;
        case clang::BO_Shl: return SSL::BinaryOp::SHL;
        case clang::BO_Shr: return SSL::BinaryOp::SHR;
        case clang::BO_And: return SSL::BinaryOp::BIT_AND;
        case clang::BO_Or: return SSL::BinaryOp::BIT_OR;
        case clang::BO_Xor: return SSL::BinaryOp::BIT_XOR;
        case clang::BO_LAnd: return SSL::BinaryOp::AND;
        case clang::BO_LOr: return SSL::BinaryOp::OR;

        case clang::BO_LT: return SSL::BinaryOp::LESS; break;
        case clang::BO_GT: return SSL::BinaryOp::GREATER; break;
        case clang::BO_LE: return SSL::BinaryOp::LESS_EQUAL; break;
        case clang::BO_GE: return SSL::BinaryOp::GREATER_EQUAL; break;
        case clang::BO_EQ: return SSL::BinaryOp::EQUAL; break;
        case clang::BO_NE: return SSL::BinaryOp::NOT_EQUAL; break;

        case clang::BO_Assign: return SSL::BinaryOp::ASSIGN;
        case clang::BO_AddAssign: return SSL::BinaryOp::ADD_ASSIGN;
        case clang::BO_SubAssign: return SSL::BinaryOp::SUB_ASSIGN;
        case clang::BO_MulAssign: return SSL::BinaryOp::MUL_ASSIGN;
        case clang::BO_DivAssign: return SSL::BinaryOp::DIV_ASSIGN;
        case clang::BO_RemAssign: return SSL::BinaryOp::MOD_ASSIGN;

        default:
            llvm::report_fatal_error("Unsupported binary operator");
    }
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
inline static clang::AnnotateAttr* IsSwizzle(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "swizzle"); }
inline static clang::AnnotateAttr* IsUnaOp(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "unaop"); }
inline static clang::AnnotateAttr* IsBinOp(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "binop"); }
inline static clang::AnnotateAttr* IsCallOp(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "builtin"); }

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
        else if (What == "half")
        {
            addType(Type, AST.HalfType);
        }
        else if (What == "buffer")
        {
            const auto& Arguments = TSD->getTemplateArgs();
            const auto* ET = Arguments.get(0).getAsType().getTypePtr();
            if (ET->isVoidType())
                addType(Type, AST.ByteBuffer((SSL::BufferFlags)SSL::BufferFlag::ReadWrite));
            else
                addType(Type, AST.StructuredBuffer(getType(ET), (SSL::BufferFlags)SSL::BufferFlag::ReadWrite));
        }
        else if (What == "image" || What == "volume")
        {

        }
        else if (What == "bindless_array")
        {
            
        }
        else if (What == "accel")
        {
            
        }
        else if (What == "ray_query_all" || What == "ray_query_any")
        {

        }
        else
        {
            ReportFatalError("Unsupported builtin type: " + std::string(What) + " for type: " + std::string(recordDecl->getName()));
            recordDecl->dump();
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
        recordFunction(x);
    return true;
}

SSL::FunctionDecl* ASTConsumer::recordFunction(const clang::FunctionDecl *x) 
{
    if (IsDump(x))
        x->dump();
    
    if (getFunc(x) || IsIgnore(x) || IsBuiltin(x))
        return nullptr;
    if (LanguageRule_UseAssignForImplicitCopyOrMove(x))
        return nullptr;

    auto CxxFunctionName = x->getQualifiedNameAsString();
    std::replace(CxxFunctionName.begin(), CxxFunctionName.end(), ':', '_');

    std::vector<SSL::ParamVarDecl*> params;
    params.reserve(x->getNumParams());
    for (const auto& param : x->parameters())
    {
        const bool isConst = param->getType().isConstQualified();
        const bool isRef = param->getType()->isReferenceType();
        const auto ParamQualType = param->getType().getCanonicalType().getNonReferenceType();
        const auto qualifier = 
            isConst ? SSL::EVariableQualifier::Const : 
            (isRef ? SSL::EVariableQualifier::Inout : SSL::EVariableQualifier::None);
        auto p = params.emplace_back(AST.DeclareParam(
            qualifier,
            getType(ParamQualType.getTypePtr()),
            ToText(param->getName()))
        );
        addVar(param, p);
    }
    auto F = AST.DeclareFunction(ToText(CxxFunctionName),
        getType(x->getReturnType().getTypePtr()),
        params,
        traverseStmt<SSL::CompoundStmt>(x->getBody())
    );
    addFunc(x, F);
    return F;
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
    ///////////////////////////////////// STMTS ///////////////////////////////////////////
    else if (auto cxxDecl = llvm::dyn_cast<clang::DeclStmt>(x)) 
    {
        const DeclGroupRef declGroup = cxxDecl->getDeclGroup();
        for (auto decl : declGroup) {
            if (!decl) continue;

            if (auto *varDecl = dyn_cast<clang::VarDecl>(decl)) 
            {
                const auto Ty = varDecl->getType();
                if (Ty->isReferenceType())
                    ReportFatalError(x, "VarDecl as reference type is not supported: [{}]", Ty.getAsString());
                if (Ty->getAsArrayTypeUnsafe())
                    ReportFatalError(x, "VarDecl as C-style array type is not supported: [{}]", Ty.getAsString());
                
                const bool isConst = varDecl->getType().isConstQualified();
                if (auto SSLType = getType(Ty.getCanonicalType().getTypePtr()))
                {
                    auto _init = traverseStmt<SSL::Expr>(varDecl->getInit());
                    auto _name = SSL::String(varDecl->getName().begin(), varDecl->getName().end());
                    auto v = AST.Variable(isConst ? SSL::EVariableQualifier::Const : SSL::EVariableQualifier::None, SSLType, _name, _init);
                    addVar(varDecl, (SSL::VarDecl*)v->decl());
                    return v; 
                } 
                else
                    ReportFatalError("VarDecl with unfound type: [{}]", Ty.getAsString());
            } else if (auto aliasDecl = dyn_cast<clang::TypeAliasDecl>(decl)) {// ignore
            } else if (auto staticAssertDecl = dyn_cast<clang::StaticAssertDecl>(decl)) {// ignore
            } else {
                ReportFatalError(x, "unsupported decl stmt: {}", cxxDecl->getStmtClassName());
            }
        }
    }
    else if (auto cxxReturn = llvm::dyn_cast<clang::ReturnStmt>(x)) 
    {
        return AST.Return(traverseStmt<SSL::Expr>(cxxReturn->getRetValue()));
    }
    ///////////////////////////////////// EXPRS ///////////////////////////////////////////
    else if (auto cxxDeclRef = llvm::dyn_cast<clang::DeclRefExpr>(x)) 
    {
        auto _cxxDecl = cxxDeclRef->getDecl();
        if (auto Function = llvm::dyn_cast<clang::FunctionDecl>(_cxxDecl))
        {
            if (!recordFunction(Function))
                ReportFatalError(x, "Function declaration with unfound type: [{}]", Function->getType().getAsString());
            return AST.Ref(getFunc(Function));
        }
        else if (auto Var = llvm::cast<clang::VarDecl>(_cxxDecl))
        {
            if (cxxDeclRef->isNonOdrUse() != NonOdrUseReason::NOUR_Unevaluated || cxxDeclRef->isNonOdrUse() != NonOdrUseReason::NOUR_Discarded) 
            {
                return AST.Ref(getVar(Var));
            }
        }
    }
    else if (auto cxxLambda = llvm::dyn_cast<LambdaExpr>(x)) 
    {
        ReportFatalError("Lambda expressions are not supported in skr-shader.");
        return nullptr;
    }
    else if (auto cxxParenExpr = llvm::dyn_cast<clang::ParenExpr>(x))
    {
        return traverseStmt<SSL::Expr>(cxxParenExpr->getSubExpr());
    }
    else if (auto cxxExplicitCast = llvm::dyn_cast<clang::ExplicitCastExpr>(x))
    {
        if (cxxExplicitCast->getType()->isFunctionPointerType())
            return traverseStmt<SSL::DeclRefExpr>(cxxExplicitCast->getSubExpr());
        auto SSLType = getType(cxxExplicitCast->getType().getTypePtr());
        if (!SSLType)
            ReportFatalError(cxxExplicitCast, "Explicit cast with unfound type: [{}]", cxxExplicitCast->getType().getAsString());
        return AST.StaticCast(SSLType, traverseStmt<SSL::Expr>(cxxExplicitCast->getSubExpr()));
    }
    else if (auto cxxImplicitCast = llvm::dyn_cast<clang::ImplicitCastExpr>(x))
    {
        if (cxxImplicitCast->getType()->isFunctionPointerType())
            return traverseStmt<SSL::DeclRefExpr>(cxxImplicitCast->getSubExpr());
        auto SSLType = getType(cxxImplicitCast->getType().getTypePtr());
        if (!SSLType)
            ReportFatalError(cxxImplicitCast, "Implicit cast with unfound type: [{}]", cxxImplicitCast->getType().getAsString());
        return AST.ImplicitCast(SSLType, traverseStmt<SSL::Expr>(cxxImplicitCast->getSubExpr()));
    }
    else if (auto cxxConstructor = llvm::dyn_cast<clang::CXXConstructExpr>(x))
    {
        auto SSLType = getType(cxxConstructor->getType().getTypePtr());
        std::vector<SSL::Expr*> _args;
        _args.reserve(cxxConstructor->getNumArgs());
        for (auto arg : cxxConstructor->arguments())
        {
            _args.emplace_back(traverseStmt<SSL::Expr>(arg));
        }
        return AST.Construct(SSLType, _args);
    }
    else if (auto cxxCall = llvm::dyn_cast<clang::CallExpr>(x))
    {
        auto funcDecl = cxxCall->getCalleeDecl();
        if (LanguageRule_UseAssignForImplicitCopyOrMove(cxxCall->getCalleeDecl()))
        {
            auto lhs = traverseStmt<SSL::Expr>(cxxCall->getArg(0));
            auto rhs = traverseStmt<SSL::Expr>(cxxCall->getArg(1));
            return AST.Assign(lhs, rhs);
        }
        else if (IsUnaOp(funcDecl) || IsBinOp(funcDecl))
        {
            return AST.Constant(IntValue(114514));
        }
        else if (IsCallOp(funcDecl))
        {
            return AST.Constant(IntValue(1919810));
        }
        else if (auto cxxMemberCall = llvm::dyn_cast<clang::CXXMemberCallExpr>(x))
        {
            auto _callee = traverseStmt<SSL::MemberExpr>(cxxMemberCall->getCallee());
            std::vector<SSL::Expr*> args;
            args.reserve(cxxMemberCall->getNumArgs());
            for (auto arg : cxxMemberCall->arguments())
            {
                args.emplace_back(traverseStmt<SSL::Expr>(arg));
            }
            return AST.CallMethod(_callee, std::span<SSL::Expr*>(args));
        }
        else
        {
            auto _callee = traverseStmt<SSL::DeclRefExpr>(cxxCall->getCallee());
            std::vector<SSL::Expr*> args;
            args.reserve(cxxCall->getNumArgs());
            for (auto arg : cxxCall->arguments())
            {
                args.emplace_back(traverseStmt<SSL::Expr>(arg));
            }
            return AST.CallFunction(_callee, args); 
        }
    }
    else if (auto cxxUnaryOp = llvm::dyn_cast<clang::UnaryOperator>(x))
    {
        const auto cxxOp = cxxUnaryOp->getOpcode();
        if (cxxOp == clang::UO_Deref)
        {
            if (auto _this = llvm::dyn_cast<CXXThisExpr>(cxxUnaryOp->getSubExpr()))
                return AST.This(); // deref 'this' (*this)
            else
                ReportFatalError(x, "Unsupported deref operator on non-'this' expression: {}", cxxUnaryOp->getStmtClassName());
        }
        else
        {
            SSL::UnaryOp op = TranslateUnaryOp(cxxUnaryOp->getOpcode());
            return AST.Unary(op, traverseStmt<SSL::Expr>(cxxUnaryOp->getSubExpr()));
        }
    }
    else if (auto cxxBinOp = llvm::dyn_cast<clang::BinaryOperator>(x))
    {
        SSL::BinaryOp op = TranslateBinaryOp(cxxBinOp->getOpcode());
        return AST.Binary(op, traverseStmt<SSL::Expr>(cxxBinOp->getLHS()), traverseStmt<SSL::Expr>(cxxBinOp->getRHS()));
    }
    else if (auto memberExpr = llvm::dyn_cast<clang::MemberExpr>(x))
    {
        auto owner = traverseStmt<SSL::DeclRefExpr>(memberExpr->getBase());
        auto memberDecl = memberExpr->getMemberDecl();
        auto functionDecl = llvm::dyn_cast<clang::FunctionDecl>(memberDecl);
        auto fieldDecl = llvm::dyn_cast<clang::FieldDecl>(memberDecl);
        if (functionDecl)
        {
            recordFunction(functionDecl);
            return AST.Method(owner, (SSL::MethodDecl*)getFunc(functionDecl));
        }
        else if (fieldDecl)
        {
            auto ownerType = getType(memberExpr->getBase()->getType().getTypePtr());
            if (IsSwizzle(fieldDecl))
            {
                auto swizzleText = fieldDecl->getName();
                uint64_t swizzle_seq[] = {0u, 0u, 0u, 0u}; /*4*/
                int64_t swizzle_size = 0;
                for (auto iter = swizzleText.begin(); iter != swizzleText.end(); iter++) {
                    if (*iter == 'x') swizzle_seq[swizzle_size] = 0u;
                    if (*iter == 'y') swizzle_seq[swizzle_size] = 1u;
                    if (*iter == 'z') swizzle_seq[swizzle_size] = 2u;
                    if (*iter == 'w') swizzle_seq[swizzle_size] = 3u;

                    if (*iter == 'r') swizzle_seq[swizzle_size] = 0u;
                    if (*iter == 'g') swizzle_seq[swizzle_size] = 1u;
                    if (*iter == 'b') swizzle_seq[swizzle_size] = 2u;
                    if (*iter == 'a') swizzle_seq[swizzle_size] = 3u;

                    swizzle_size += 1;
                }
                return AST.Swizzle(owner, swizzle_size, swizzle_seq);
            }
            else if (!fieldDecl->isAnonymousStructOrUnion())
            {
                auto memberName = ToText(memberExpr->getMemberNameInfo().getName().getAsString());
                if (memberName.empty())
                    ReportFatalError(x, "Member name is empty in member expr: {}", memberExpr->getStmtClassName());
                return AST.Field(owner, ownerType->get_field(memberName));
            }
            else
            {
                return owner;
            }
        }
        else 
        {
            ReportFatalError(x, "unsupported member expr: {}", memberExpr->getStmtClassName());
        }
    }
    else if (auto matTemp = llvm::dyn_cast<clang::MaterializeTemporaryExpr>(x))
    {
        return traverseStmt(matTemp->getSubExpr());
    }
    else if (auto THIS = llvm::dyn_cast<clang::CXXThisExpr>(x))
    {
        return AST.This();
    }
    else if (auto INT = llvm::dyn_cast<clang::IntegerLiteral>(x))
    {
        return AST.Constant(SSL::IntValue(INT->getValue().getLimitedValue()));
    }
    else if (auto FLOAT = llvm::dyn_cast<clang::FloatingLiteral>(x))
    {
        return AST.Constant(SSL::FloatValue(FLOAT->getValue().convertToFloat()));
    }

    ReportFatalError(x, "unsupported stmt: {}", x->getStmtClassName());
    return nullptr;
}

bool ASTConsumer::addVar(const clang::VarDecl* var, skr::SSL::VarDecl* _var)
{
    if (!_vars.emplace(var, _var).second)
    {
        ReportFatalError("Duplicate variable declaration: " + std::string(var->getName()));
        return false;
    }
    return true;
}

skr::SSL::VarDecl* ASTConsumer::getVar(const clang::VarDecl* var) const
{
    auto it = _vars.find(var);
    if (it != _vars.end())
        return it->second;

    var->dump();
    ReportFatalError("DeclRefExpr with unfound variable: [{}]", var->getNameAsString());
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

bool ASTConsumer::addFunc(const clang::FunctionDecl* func, skr::SSL::FunctionDecl* decl)
{
    if (!_funcs.emplace(func, decl).second)
    {
        ReportFatalError("Duplicate function declaration: " + std::string(func->getName()));
        return false;
    }
    return true;
}

skr::SSL::FunctionDecl* ASTConsumer::getFunc(const clang::FunctionDecl* func) const
{
    auto it = _funcs.find(func);
    if (it != _funcs.end())
        return it->second;
    return nullptr;
}

} // namespace skr::SSL