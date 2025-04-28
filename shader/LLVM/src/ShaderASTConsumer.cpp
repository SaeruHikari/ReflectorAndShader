#include "DebugASTVisitor.hpp"
#include "ShaderASTConsumer.hpp"
#include <clang/Frontend/CompilerInstance.h>

#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>
#include <clang/AST/DeclTemplate.h>

namespace skr::SSL {

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
        static_assert(false, "Unsupported type for GetArgumentAt");
    }
}

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

    auto attrs = TST ? TST->specific_attrs<clang::AnnotateAttr>() : x->specific_attrs<clang::AnnotateAttr>();
    for (auto attr : attrs)
    {
        if (attr->getAnnotation() != "skr-shader")
            continue;
        
        auto name = GetArgumentAt<clang::StringRef>(attr, 0);
        if (name == "ignore") 
            return true;
        if (name == "builtin") 
            return true;
    }

    // llvm::outs() << x->getName() << "\n";

    return true;
}

} // namespace skr::SSL