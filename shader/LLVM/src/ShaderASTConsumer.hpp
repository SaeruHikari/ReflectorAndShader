#pragma once
#include <clang/AST/ASTConsumer.h>
#include <clang/Tooling/Tooling.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace skr::SSL {

struct CompileFrontendAction : public clang::ASTFrontendAction 
{
public:
    CompileFrontendAction();
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) final;
};

struct FunctionDeclStmtHandler : public clang::ast_matchers::MatchFinder::MatchCallback {
    FunctionDeclStmtHandler() = default;
    void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) final;
};

class ASTConsumer : public clang::ASTConsumer 
{
public:
    explicit ASTConsumer();
    virtual ~ASTConsumer() override;

    void HandleTranslationUnit(clang::ASTContext &Context) override;
    clang::ast_matchers::MatchFinder Matcher;
    FunctionDeclStmtHandler HandlerForFuncionDecl;
};
    
}