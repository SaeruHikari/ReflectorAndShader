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
    SSL::FunctionDecl* recordFunction(const clang::FunctionDecl* x);

    template <typename T>
    T* traverseStmt(const clang::Stmt* x);
    Stmt* traverseStmt(const clang::Stmt *x);
    
    bool addType(const clang::Type* type, const skr::SSL::TypeDecl* decl);
    bool addType(const clang::Type* type, skr::SSL::TypeDecl* decl);
    skr::SSL::TypeDecl* getType(const clang::Type* type) const;
    bool addVar(const clang::VarDecl* var, skr::SSL::VarDecl* decl);
    skr::SSL::VarDecl* getVar(const clang::VarDecl* var) const;
    bool addFunc(const clang::FunctionDecl* func, skr::SSL::FunctionDecl* decl);
    skr::SSL::FunctionDecl* getFunc(const clang::FunctionDecl* func) const;
    
    clang::ASTContext* pASTContext = nullptr;
    std::map<const clang::TagDecl*, skr::SSL::TypeDecl*> _tag_types;
    std::map<const clang::BuiltinType::Kind, skr::SSL::TypeDecl*> _builtin_types;
    std::map<const clang::VarDecl*, skr::SSL::VarDecl*> _vars;
    std::map<const clang::FunctionDecl*, skr::SSL::FunctionDecl*> _funcs;
    AST& AST;

protected:
    void DumpWithLocation(const clang::Stmt *stmt) const;
    void ReportFatalError(const std::string& message) const;
    template <typename... Args>
    void ReportFatalError(std::format_string<Args...> _fmt, Args&&... args) const;
    template <typename... Args>
    void ReportFatalError(const clang::Stmt* expr, std::format_string<Args...> _fmt, Args&&... args) const;
};
    
} // namespace skr::SSL