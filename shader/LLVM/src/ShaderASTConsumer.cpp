#include "DebugASTVisitor.hpp"
#include "ShaderASTConsumer.hpp"
#include <clang/Frontend/CompilerInstance.h>

#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>
#include <clang/AST/DeclTemplate.h>

namespace skr::SSL {

inline static void ReportFatalError(const std::string& message)
{
    llvm::report_fatal_error(message.c_str());
}

inline static String ToString(clang::StringRef str)
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

void ASTConsumer::HandleTranslationUnit(clang::ASTContext &Context)
{
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
        skr::SSL::String Name(VarName.begin(), VarName.end());
        AST.DeclareGlobalConstant(UnderlyingType, Name, AST.Constant(IntValue(I)));
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

    auto NewType = AST.DeclareType(ToString(recordDecl->getName()), {});
    for (auto field : recordDecl->fields())
    {
        if (IsDump(field)) 
            field->dump();

        auto desugaredFTy = field->getType().getCanonicalType();
        auto fieldType = desugaredFTy->getAs<clang::Type>();

        if (auto ft = getType(fieldType))
        {
            std::string_view name_v = field->getName();
            skr::SSL::String name(name_v.begin(), name_v.end());
            NewType->add_field(AST.DeclareField(name, ft));
        }
        else
        {
            ReportFatalError("Unknown field type: " + std::string(fieldType->getTypeClassName()) + " for field: " + field->getName().str());
        }
    } 
    addType(Type, NewType);
    return true;
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