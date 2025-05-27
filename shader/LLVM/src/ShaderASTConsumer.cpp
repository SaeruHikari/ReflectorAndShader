#include "DebugASTVisitor.hpp"
#include "ShaderASTConsumer.hpp"
#include <clang/Frontend/CompilerInstance.h>

#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>
#include <clang/AST/DeclTemplate.h>
#include "SSL/TestASTVisitor.hpp"

namespace skr::SSL {

inline static String ToString(clang::StringRef str)
{
    return String(str.begin(), str.end());
}

template <typename T>
inline static T GetArgumentAt(clang::AnnotateAttr* attr, size_t index)
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

inline static clang::AnnotateAttr* ExistShaderAttrWithName(clang::Decl* decl, const char* name)
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

inline static clang::AnnotateAttr* IsIgnore(clang::Decl* decl) { return ExistShaderAttrWithName(decl, "ignore"); }
inline static clang::AnnotateAttr* IsBuiltin(clang::Decl* decl) { return ExistShaderAttrWithName(decl, "builtin"); }

CompileFrontendAction::CompileFrontendAction()
    : clang::ASTFrontendAction()
{
}

std::unique_ptr<clang::ASTConsumer> CompileFrontendAction::CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile)
{
    auto &LO = CI.getLangOpts();
    LO.CommentOpts.ParseAllComments = true;
    return std::make_unique<skr::SSL::ASTConsumer>();
}

ASTConsumer::ASTConsumer()
    : clang::ASTConsumer()
{

}

ASTConsumer::~ASTConsumer()
{

}

void ASTConsumer::HandleTranslationUnit(clang::ASTContext &Context)
{
    // Context.getTranslationUnitDecl()->dump();

    DebugASTVisitor debug = {};
    debug.TraverseDecl(Context.getTranslationUnitDecl());
    TraverseDecl(Context.getTranslationUnitDecl());
}

bool ASTConsumer::VisitRecordDecl(clang::RecordDecl* x)
{
    const auto* TST = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(x);
    if (x->isUnion()) return true; // unions are not supported
    if (!x->isCompleteDefinition()) return true; // skip forward declares
    if (TST && !TST->hasDefinition()) return true; // skip no-def template specs
    if (IsIgnore(x)) return true; // skip ignored types

    if (auto BuiltinAttr = IsBuiltin(x))
    {
        llvm::outs() << x->getName() << "\n";
        return true;
    } 
    /*
        for (auto field : x->fields())
        {
            // field->getType()
            // AST.Field(field->getName(), field->);
        }
    */
    AST.DeclareType(ToString(x->getName()), {});
    // llvm::outs() << x->getName() << "\n";

    return true;
}

} // namespace skr::SSL