#include "SSL/magic_enum/magic_enum.hpp"
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

template <typename... Args>
inline void ASTConsumer::ReportFatalError(const clang::Decl* decl, std::format_string<Args...> _fmt, Args&&... args) const
{
    DumpWithLocation(decl);
    ReportFatalError(_fmt, std::forward<Args>(args)...);
}

void ASTConsumer::DumpWithLocation(const clang::Stmt *stmt) const
{
    stmt->getBeginLoc().dump(pASTContext->getSourceManager());
    stmt->dump();
}

void ASTConsumer::DumpWithLocation(const clang::Decl* decl) const
{
    decl->getBeginLoc().dump(pASTContext->getSourceManager());
    decl->dump();
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
    else if constexpr (std::is_integral_v<T>)
    {
        auto arg = llvm::dyn_cast<clang::IntegerLiteral>((*args)->IgnoreParenCasts());
        return arg->getValue().getLimitedValue();
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
inline static clang::AnnotateAttr* IsCallOp(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "callop"); }
inline static clang::AnnotateAttr* IsAccess(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "access"); }
inline static clang::AnnotateAttr* IsStage(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "stage"); }

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
    constexpr auto bin_op_count = (uint64_t)BinaryOp::NOT_EQUAL + 1u;
    for (uint32_t i = 0; i < bin_op_count; i++) 
    {
        const auto op = (BinaryOp)i;
        _bin_ops.emplace(magic_enum::enum_name(op), op);
    }
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
                const skr::SSL::TypeDecl* Types[] = { AST.Float2Type, AST.Float3Type, AST.Float4Type };
                addType(Type, Types[N - 2]);
            }
            else if (getType(ET) == AST.IntType)
            {
                const skr::SSL::TypeDecl* Types[] = { AST.Int2Type, AST.Int3Type, AST.Int4Type };
                addType(Type, Types[N - 2]);
            }
            else if (getType(ET) == AST.UIntType)
            {
                const skr::SSL::TypeDecl* Types[] = { AST.UInt2Type, AST.UInt3Type, AST.UInt4Type };
                addType(Type, Types[N - 2]);
            }
            else if (getType(ET) == AST.BoolType)
            {
                const skr::SSL::TypeDecl* Types[] = { AST.Bool2Type, AST.Bool3Type, AST.Bool4Type };
                addType(Type, Types[N - 2]);
            }
            else
            {
                ReportFatalError(recordDecl, "Unsupported vec type: {} for vec size: {}", std::string(ET->getTypeClassName()), std::to_string(N));
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
            const skr::SSL::TypeDecl* Types[] = { AST.Float2x2Type, AST.Float3x3Type, AST.Float4x4Type };
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
            ReportFatalError("Unsupported builtin type: {} for type: {}", std::string(What), std::string(recordDecl->getName()));
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
    if (auto StageInfo = IsStage(x))
    {
        auto StageName = GetArgumentAt<clang::StringRef>(StageInfo, 1);
        auto FunctionName = GetArgumentAt<clang::StringRef>(StageInfo, 2);

        if (StageName == "compute")
        {
            if (auto KernelInfo = IsKernel(x))
            {
                auto Kernel = recordFunction(x, FunctionName);
                Kernel->add_attr(AST.DeclareAttr<StageAttr>(ShaderStage::Compute));

                uint32_t KernelX = GetArgumentAt<uint32_t>(KernelInfo, 1);
                uint32_t KernelY = GetArgumentAt<uint32_t>(KernelInfo, 2);
                uint32_t KernelZ = GetArgumentAt<uint32_t>(KernelInfo, 3);
                Kernel->add_attr(AST.DeclareAttr<KernelSizeAttr>(KernelX, KernelY, KernelZ));
            }
            else
                ReportFatalError("Compute shader function must have kernel size attributes: " + std::string(x->getNameAsString()));
        }
        else
        {
            ReportFatalError(x, "Unsupported stage function: {}", std::string(x->getNameAsString()));
        }
    }
    return true;
}

bool ASTConsumer::VisitVarDecl(clang::VarDecl* x)
{
    if (IsDump(x))
        x->dump();

    if (x->hasExternalStorage())
    {
        auto ResourceType = x->getType().getNonReferenceType().getCanonicalType().getTypePtr();
        auto ShaderResource = AST.DeclareGlobalResource(getType(ResourceType), ToText(x->getName()));
        addVar(x, ShaderResource);
    }
    return true;
}

SSL::FunctionDecl* ASTConsumer::recordFunction(const clang::FunctionDecl *x, llvm::StringRef override_name) 
{
    if (IsDump(x))
        x->dump();
    
    if (auto Existed = getFunc(x))
        return Existed;
    if (IsIgnore(x) || IsBuiltin(x))
        return nullptr;
    if (LanguageRule_UseAssignForImplicitCopyOrMove(x))
        return nullptr;

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
        auto _param = params.emplace_back(AST.DeclareParam(
            qualifier,
            getType(ParamQualType.getTypePtr()),
            ToText(param->getName()))
        );
        addVar(param, _param);

        if (auto BuiltinInfo = IsBuiltin(param))
        {
            auto BuiltinName = GetArgumentAt<clang::StringRef>(BuiltinInfo, 1);
            _param->add_attr(AST.DeclareAttr<BuiltinAttr>(ToText(BuiltinName)));
        }
    }

    SSL::FunctionDecl* F = nullptr;
    auto AsMethod = llvm::dyn_cast<clang::CXXMethodDecl>(x);
    if (AsMethod && !AsMethod->isStatic())
    {
        auto ownerType = getType(AsMethod->getParent()->getTypeForDecl());
        if (auto AsCtor = llvm::dyn_cast<clang::CXXConstructorDecl>(AsMethod))
        {
            if (ownerType->is_builtin())
                return nullptr;

            auto body = AST.Block({});
            for (auto ctor_init : AsCtor->inits())
            {
                if (auto F = ctor_init->getMember())
                {
                    auto I = ctor_init->getMember()->getFieldIndex();
                    auto N = ToText(F->getDeclName().getAsString());
                    body->add_statement(
                        AST.Assign(
                            AST.Field(AST.This(ownerType), ownerType->get_field(N)),
                            (SSL::Expr*)traverseStmt(ctor_init->getInit())
                        )
                    );
                }
                else
                {
                    ReportFatalError(x, "Derived class is currently unsupported!");
                }
            }
            
            if (auto func = traverseStmt<SSL::CompoundStmt>(x->getBody()))
                body->add_statement(func);

            F = AST.DeclareConstructor(
                ownerType,
                ConstructorDecl::kSymbolName,
                params,
                body
            );
            ownerType->add_ctor((SSL::ConstructorDecl*)F);
        }
        else
        {
            F = AST.DeclareMethod(
                ownerType,
                ToText(AsMethod->getName()),
                getType(x->getReturnType().getTypePtr()),
                params,
                traverseStmt<SSL::CompoundStmt>(x->getBody())
            );
            ownerType->add_method((SSL::MethodDecl*)F);
        }
    }
    else
    {
        auto CxxFunctionName = override_name.empty() ? x->getQualifiedNameAsString() : override_name.str();
        std::replace(CxxFunctionName.begin(), CxxFunctionName.end(), ':', '_');
        F = AST.DeclareFunction(ToText(CxxFunctionName),
            getType(x->getReturnType().getTypePtr()),
            params,
            traverseStmt<SSL::CompoundStmt>(x->getBody())
        );
    }
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
                    return traverseStmt(cxxBranch->getThen());
            } else {
                if (cxxBranch->getElse())
                    return traverseStmt(cxxBranch->getElse());
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
            return AST.Ref(getFunc(Function));
        }
        else if (auto Var = llvm::cast<clang::VarDecl>(_cxxDecl))
        {
            if (Var->isConstexpr())
            {
                if (cxxDeclRef->isNonOdrUse() != NonOdrUseReason::NOUR_Unevaluated || cxxDeclRef->isNonOdrUse() != NonOdrUseReason::NOUR_Discarded) 
                {
                    if (auto Decompressed = Var->getPotentiallyDecomposedVarDecl())
                    {
                        if (auto Evaluated = Decompressed->getEvaluatedValue()) 
                        {
                            if (Evaluated->isInt())
                            {
                                return AST.Constant(IntValue(Evaluated->getInt().getLimitedValue()));
                            }
                            else if (Evaluated->isFloat())
                            {
                                return AST.Constant(FloatValue(Evaluated->getFloat().convertToDouble()));
                            }
                            else 
                            {
                                ReportFatalError(x, "!!!!");
                            }
                        }
                    }
                }
            }
            else
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
        if (!SSLType->is_builtin())
        {
            recordFunction(llvm::dyn_cast<clang::FunctionDecl>(cxxConstructor->getConstructor()));
        }
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
        else if (auto AsUnaOp = IsUnaOp(funcDecl))
        {
            auto name = GetArgumentAt<clang::StringRef>(AsUnaOp, 1);
            if (name == "PLUS")
                return AST.Unary(SSL::UnaryOp::PLUS, traverseStmt<SSL::Expr>(cxxCall->getArg(0)));
            else if (name == "MINUS")
                return AST.Unary(SSL::UnaryOp::MINUS, traverseStmt<SSL::Expr>(cxxCall->getArg(0)));
            else if (name == "NOT")
                return AST.Unary(SSL::UnaryOp::NOT, traverseStmt<SSL::Expr>(cxxCall->getArg(0)));
            else if (name == "BIT_NOT")
                return AST.Unary(SSL::UnaryOp::BIT_NOT, traverseStmt<SSL::Expr>(cxxCall->getArg(0)));
            else if (name == "PRE_INC")
                return AST.Unary(SSL::UnaryOp::PRE_INC, traverseStmt<SSL::Expr>(cxxCall->getArg(0)));
            else if (name == "PRE_DEC")
                return AST.Unary(SSL::UnaryOp::PRE_DEC, traverseStmt<SSL::Expr>(cxxCall->getArg(0)));
            else if (name == "POST_INC")
                return AST.Unary(SSL::UnaryOp::POST_INC, traverseStmt<SSL::Expr>(cxxCall->getArg(0)));
            else if (name == "POST_DEC")
                return AST.Unary(SSL::UnaryOp::POST_DEC, traverseStmt<SSL::Expr>(cxxCall->getArg(0)));
            ReportFatalError(x, "Unsupported unary operator: {}", name.str());
        }
        else if (auto AsBinOp = IsBinOp(funcDecl))
        {
            auto name = GetArgumentAt<clang::StringRef>(AsBinOp, 1);
            auto&& iter = _bin_ops.find(name.str());
            if (iter == _bin_ops.end())
                ReportFatalError(x, "Unsupported binary operator: {}", name.str());
            SSL::BinaryOp op = iter->second;
            auto lhs = traverseStmt<SSL::Expr>(cxxCall->getArg(0));
            auto rhs = traverseStmt<SSL::Expr>(cxxCall->getArg(1));
            return AST.Binary(op, lhs, rhs);
        }
        else if (IsAccess(funcDecl))
        {
            ReportFatalError(x, "Array access operator is not supported yet: {}", cxxCall->getStmtClassName());
        }
        else if (auto AsCallOp = IsCallOp(funcDecl))
        {
            auto name = GetArgumentAt<clang::StringRef>(AsCallOp, 1);
            if (auto Intrin = AST.FindIntrinsic(name.data()))
            {
                const bool IsMethod = llvm::dyn_cast<clang::CXXMemberCallExpr>(cxxCall);
                std::vector<const TypeDecl*> arg_types;
                std::vector<EVariableQualifier> arg_qualifiers;
                std::vector<SSL::Expr*> args;
                args.reserve(cxxCall->getNumArgs() + (IsMethod ? 1 : 0));
                arg_types.reserve(cxxCall->getNumArgs() + (IsMethod ? 1 : 0));
                arg_qualifiers.reserve(cxxCall->getNumArgs() + (IsMethod ? 1 : 0));
                if (IsMethod)
                {
                    auto _clangMember = llvm::dyn_cast<clang::MemberExpr>(llvm::dyn_cast<clang::CXXMemberCallExpr>(x)->getCallee());
                    auto _caller = traverseStmt<SSL::DeclRefExpr>(_clangMember->getBase());
                    arg_types.emplace_back(_caller->type());
                    arg_qualifiers.emplace_back(EVariableQualifier::Inout);
                    args.emplace_back(_caller);
                }
                for (size_t i = 0; i < cxxCall->getNumArgs(); ++i)
                {
                    arg_types.emplace_back(getType(cxxCall->getArg(i)->getType().getTypePtr()));
                    arg_qualifiers.emplace_back(EVariableQualifier::None);
                    args.emplace_back(traverseStmt<SSL::Expr>(cxxCall->getArg(i)));
                }
                // TODO: CACHE THIS
                if (auto Spec = AST.SpecializeTemplateFunction(Intrin, arg_types, arg_qualifiers))
                    return AST.CallFunction(Spec->ref(), args);
                else
                    ReportFatalError(x, "Failed to specialize template function: {}", name.str());
            }
            else
                ReportFatalError(x, "Unsupported call operator: {}", name.str());
        }
        else if (auto cxxMemberCall = llvm::dyn_cast<clang::CXXMemberCallExpr>(x))
        {
            if (!recordFunction(llvm::dyn_cast<clang::FunctionDecl>(funcDecl)))
                ReportFatalError(x, "Method declaration failed!");

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
            if (!recordFunction(llvm::dyn_cast<clang::FunctionDecl>(funcDecl)))
                ReportFatalError(x, "Function declaration failed!");

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
                return AST.This(getType(_this->getType().getCanonicalType().getTypePtr())); // deref 'this' (*this)
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
        auto methodDecl = llvm::dyn_cast<clang::CXXMethodDecl>(memberDecl);
        auto fieldDecl = llvm::dyn_cast<clang::FieldDecl>(memberDecl);
        if (methodDecl)
        {
            return AST.Method(owner, (SSL::MethodDecl*)getFunc(methodDecl));
        }
        else if (fieldDecl)
        {
            if (IsSwizzle(fieldDecl))
            {
                auto swizzleResultType = getType(fieldDecl->getType().getTypePtr());
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
                return AST.Swizzle(owner, swizzleResultType, swizzle_size, swizzle_seq);
            }
            else if (!fieldDecl->isAnonymousStructOrUnion())
            {
                auto ownerType = getType(fieldDecl->getParent()->getTypeForDecl());
                if (!ownerType)
                    ReportFatalError(x, "Member expr with unfound owner type: [{}]", memberExpr->getBase()->getType().getAsString());
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
        return AST.This(getType(THIS->getType().getCanonicalType().getTypePtr()));
    }
    else if (auto InitExpr = llvm::dyn_cast<CXXDefaultInitExpr>(x))
    {
        return traverseStmt(InitExpr->getExpr());
    }
    else if (auto INT = llvm::dyn_cast<clang::IntegerLiteral>(x))
    {
        return AST.Constant(SSL::IntValue(INT->getValue().getLimitedValue()));
    }
    else if (auto FLOAT = llvm::dyn_cast<clang::FloatingLiteral>(x))
    {
        return AST.Constant(SSL::FloatValue(FLOAT->getValue().convertToFloat()));
    }
    else if (auto cxxNullStmt = llvm::dyn_cast<clang::NullStmt>(x))
    {
        return AST.Block({});
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

    ReportFatalError(var, "DeclRefExpr with unfound variable: [{}]", var->getNameAsString());
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

bool ASTConsumer::addType(const clang::Type* type, const skr::SSL::TypeDecl* decl)
{
    return addType(type, const_cast<skr::SSL::TypeDecl*>(decl));
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