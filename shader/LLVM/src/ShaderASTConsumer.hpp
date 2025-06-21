#pragma once
#include <clang/AST/ASTConsumer.h>
#include <clang/Tooling/Tooling.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include "SSL/AST.hpp"

namespace skr::SSL {

struct CompileFrontendAction : public clang::ASTFrontendAction 
{
public:
    CompileFrontendAction(skr::SSL::AST& AST);
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) final;
    skr::SSL::AST& AST;
};

class ASTConsumer : public clang::ASTConsumer, public clang::RecursiveASTVisitor<ASTConsumer>
{
public:
    explicit ASTConsumer(skr::SSL::AST& AST);
    virtual ~ASTConsumer() override;

    void HandleTranslationUnit(clang::ASTContext &Context) override;

public:
    // ASTVisitor
    bool shouldVisitTemplateInstantiations() const { return true; }
    bool VisitEnumDecl(clang::EnumDecl* x);
    bool VisitRecordDecl(clang::RecordDecl* x);
    bool VisitFunctionDecl(clang::FunctionDecl* x);

protected:
    template <typename T>
    T* traverseStmt(const clang::Stmt* x);

    Stmt* traverseStmt(const clang::Stmt *x);
    bool addType(const clang::Type* type, skr::SSL::TypeDecl* decl);
    skr::SSL::TypeDecl* getType(const clang::Type* type) const;
    
    const clang::ASTContext* pASTContext = nullptr;
    std::map<const clang::TagDecl*, skr::SSL::TypeDecl*> _tag_types;
    std::map<const clang::BuiltinType::Kind, skr::SSL::TypeDecl*> _builtin_types;
    AST& AST;

protected:
    void DumpWithLocation(const clang::Stmt *stmt);
    void ReportFatalError(const std::string& message);
    template <typename... Args>
    void ReportFatalError(std::format_string<Args...> _fmt, Args&&... args);
    template <typename... Args>
    void ReportFatalError(const clang::Stmt* expr, std::format_string<Args...> _fmt, Args&&... args);
};
    
} // namespace skr::SSL