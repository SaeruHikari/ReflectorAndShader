#pragma once
#include "AST.hpp"
#include "TestASTVisitor.hpp"

namespace skr::SSL {

struct ASTDumper
{
public:
    skr::SSL::String Visit(const skr::SSL::Decl* decl);
    skr::SSL::String Visit(const skr::SSL::Stmt* stmt);

protected:
    skr::SSL::String visit(const skr::SSL::Stmt* stmt);
    skr::SSL::String visit(const skr::SSL::TypeDecl* typeDecl);
    skr::SSL::String visit(const skr::SSL::FieldDecl* fieldDecl);
    skr::SSL::String visit(const skr::SSL::ParamVarDecl* paramDecl);
    skr::SSL::String visit(const skr::SSL::FunctionDecl* funcDecl);
    skr::SSL::String visit(const skr::SSL::VarDecl* varDecl);

    SourceBuilder sb = SourceBuilder(L"| ");
};

} // namespace skr::SSL