#include "ExpressionTranslator.hpp"
#include "ShaderASTConsumer.hpp"
#include <clang/Frontend/CompilerInstance.h>

#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>
#include <clang/AST/DeclTemplate.h>

namespace skr::SSL {

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
    Context.getTranslationUnitDecl()->dump();

    ExprTranslator expr = {};
    expr.TraverseDecl(Context.getTranslationUnitDecl());
}

}