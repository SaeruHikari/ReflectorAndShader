#include "ShaderASTConsumer.hpp"
#include <clang/Frontend/CompilerInstance.h>

#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
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

void FunctionDeclStmtHandler::run(const clang::ast_matchers::MatchFinder::MatchResult &Result) {
    // The matched 'if' statement was bound to 'ifStmt'.
    if (const auto *S = Result.Nodes.getNodeAs<clang::FunctionDecl>("FunctionDecl")) {
        S->dump();
    }
}


ASTConsumer::ASTConsumer()
    : clang::ASTConsumer()
{
    using namespace clang::ast_matchers;

    Matcher.addMatcher(functionDecl(
        isDefinition(),
        unless(isExpansionInSystemHeader()))
        .bind("FunctionDecl"),
        &HandlerForFuncionDecl
    );
}

ASTConsumer::~ASTConsumer()
{

}

void ASTConsumer::HandleTranslationUnit(clang::ASTContext &Context)
{
    Matcher.matchAST(Context);
}

}