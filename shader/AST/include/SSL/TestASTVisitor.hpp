#pragma once
#include "AST.hpp"
#include "SourceBuilder.hpp"

namespace skr::SSL
{
struct ASTVisitor
{
    skr::SSL::String visitExpr(const skr::SSL::Stmt* stmt);
    skr::SSL::String visit(const skr::SSL::TypeDecl* typeDecl);
    skr::SSL::String visit(const skr::SSL::FunctionDecl* funcDecl);
};
} // namespace skr::SSL