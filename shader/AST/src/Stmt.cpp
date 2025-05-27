#include "SSL/Stmt.hpp"
#include "SSL/AST.hpp"

namespace skr::SSL {

Stmt::Stmt(const AST& ast) : _ast(&ast) {}

DeclStmt::DeclStmt(const AST& ast, Decl* decl) : Stmt(ast), _decl(decl) {}

DeclRefExpr* DeclStmt::ref() const
{
    return const_cast<AST*>(_ast)->Ref(decl());
}

CompoundStmt::CompoundStmt(const AST& ast, std::span<Stmt* const> statements) 
    : Stmt(ast)
{
    for (auto& statement : statements)
    {
        _children.emplace_back(statement);
    }
}

void CompoundStmt::add_statement(Stmt* statement)
{
    _children.emplace_back(statement);
}

ValueStmt::ValueStmt(const AST& ast)
    : Stmt(ast)
{

}

} // namespace skr::SSL