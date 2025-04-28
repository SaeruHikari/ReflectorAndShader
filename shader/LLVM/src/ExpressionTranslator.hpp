#pragma once
#include <clang/AST/RecursiveASTVisitor.h>

namespace skr::SSL {

struct ExprTranslator : public clang::RecursiveASTVisitor<ExprTranslator>
{
    bool shouldVisitTemplateInstantiations() const { return true; }
    bool VisitStmt(clang::Stmt* x);
    bool VisitRecordDecl(clang::RecordDecl* x);
    bool VisitFunctionDecl(clang::FunctionDecl* x);
};

} // namespace skr::SSL