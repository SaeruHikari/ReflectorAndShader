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
    // ASTVisitor APIs
    bool shouldVisitTemplateInstantiations() const { return true; }
    bool VisitEnumDecl(const clang::EnumDecl* x);
    bool VisitRecordDecl(const clang::RecordDecl* x);
    bool VisitFunctionDecl(const clang::FunctionDecl* x);
    bool VisitFieldDecl(const clang::FieldDecl* x);
    bool VisitVarDecl(const clang::VarDecl* x);

protected:
    SSL::TypeDecl* TranslateType(clang::QualType type);
    SSL::TypeDecl* TranslateRecordDecl(const clang::RecordDecl* x);
    SSL::TypeDecl* TranslateEnumDecl(const clang::EnumDecl* x);
    SSL::FunctionDecl* TranslateFunction(const clang::FunctionDecl* x, llvm::StringRef override_name = {});

    Stmt* TranslateStmt(const clang::Stmt *x);
    template <typename T>
    T* TranslateStmt(const clang::Stmt* x);
    
    bool addType(clang::QualType type, const skr::SSL::TypeDecl* decl);
    bool addType(clang::QualType type, skr::SSL::TypeDecl* decl);
    skr::SSL::TypeDecl* getType(clang::QualType type) const;
    bool addVar(const clang::VarDecl* var, skr::SSL::VarDecl* decl);
    skr::SSL::VarDecl* getVar(const clang::VarDecl* var) const;
    bool addFunc(const clang::FunctionDecl* func, skr::SSL::FunctionDecl* decl);
    skr::SSL::FunctionDecl* getFunc(const clang::FunctionDecl* func) const;
    
    clang::ASTContext* pASTContext = nullptr;
    std::map<const clang::TagDecl*, skr::SSL::TypeDecl*> _tag_types;
    std::map<const clang::BuiltinType::Kind, skr::SSL::TypeDecl*> _builtin_types;
    std::map<const clang::VarDecl*, skr::SSL::VarDecl*> _vars;
    std::map<const clang::FunctionDecl*, skr::SSL::FunctionDecl*> _funcs;
    std::map<const clang::EnumConstantDecl*, skr::SSL::GlobalVarDecl*> _enum_constants;
    uint64_t next_lambda_id = 0;
    uint64_t next_template_spec_id = 0;
    AST& AST;
    
protected:
    void DumpWithLocation(const clang::Stmt *stmt) const;
    void DumpWithLocation(const clang::Decl *decl) const;
    void ReportFatalError(const std::string& message) const;
    template <typename... Args>
    void ReportFatalError(std::format_string<Args...> _fmt, Args&&... args) const;
    template <typename... Args>
    void ReportFatalError(const clang::Stmt* expr, std::format_string<Args...> _fmt, Args&&... args) const;
    template <typename... Args>
    void ReportFatalError(const clang::Decl* decl, std::format_string<Args...> _fmt, Args&&... args) const;
    
    std::map<std::string, skr::SSL::BinaryOp> _bin_ops;
};
    
} // namespace skr::SSL