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

CompoundStmt* AST::Block(const std::vector<Stmt*>& statements)
{
    auto exp = new CompoundStmt(*this, statements);
    _stmts.emplace_back(exp);
    return exp;
}

ValueStmt::ValueStmt(const AST& ast)
    : Stmt(ast)
{

}

} // namespace skr::SSL