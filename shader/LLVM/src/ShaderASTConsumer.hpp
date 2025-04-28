#pragma once
#include <clang/AST/ASTConsumer.h>
#include <clang/Tooling/Tooling.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include "SSL/AST.hpp"

namespace skr::SSL {

struct CompileFrontendAction : public clang::ASTFrontendAction 
{
public:
    CompileFrontendAction();
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) final;
};

class ASTConsumer : public clang::ASTConsumer, public clang::RecursiveASTVisitor<ASTConsumer>
{
public:
    explicit ASTConsumer();
    virtual ~ASTConsumer() override;

    void HandleTranslationUnit(clang::ASTContext &Context) override;

public:
    // ASTVisitor
    bool shouldVisitTemplateInstantiations() const { return true; }
    bool VisitRecordDecl(clang::RecordDecl* x);

protected:
    AST AST;
};
    
}