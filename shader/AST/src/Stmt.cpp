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

IfStmt::IfStmt(const AST& ast, Stmt* cond, CompoundStmt* then_body, CompoundStmt* else_body)
    : Stmt(ast), _cond(cond), _then_body(then_body), _else_body(else_body)
{
    _children.emplace_back(cond);
    _children.emplace_back(then_body);
    if (else_body) {
        _children.emplace_back(else_body);
    }
}

ForStmt::ForStmt(const AST& ast, Stmt* init, Stmt* cond, Stmt* inc, CompoundStmt* body)
    : Stmt(ast), _init(init), _cond(cond), _inc(inc), _body(body)
{
    _children.emplace_back(init);
    _children.emplace_back(cond);
    _children.emplace_back(inc);
    _children.emplace_back(body);
}

WhileStmt::WhileStmt(const AST& ast, Stmt* cond, CompoundStmt* body)
    : Stmt(ast), _cond(cond), _body(body)
{
    _children.emplace_back(cond);
    _children.emplace_back(body);
}

BreakStmt::BreakStmt(const AST& ast)
    : Stmt(ast)
{

}

ContinueStmt::ContinueStmt(const AST& ast)
    : Stmt(ast)
{

}

DefaultStmt::DefaultStmt(const AST& ast)
    : Stmt(ast)
{

}

SwitchStmt::SwitchStmt(const AST& ast, Stmt* cond, std::span<CaseStmt*> cases)
    : Stmt(ast), _cond(cond), _cases(cases.begin(), cases.end())
{
    _children.emplace_back(cond);
    for (auto case_stmt : _cases)
    {
        _children.emplace_back(case_stmt);
    }
}

CaseStmt::CaseStmt(const AST& ast, Stmt* cond, Stmt* body)
    : Stmt(ast), _cond(cond), _body(body)
{
    _children.emplace_back(cond);
    _children.emplace_back(body);
}

ReturnStmt::ReturnStmt(const AST& ast, Stmt* value)
    : Stmt(ast), _value(value)
{
    _children.emplace_back(value);
}

ValueStmt::ValueStmt(const AST& ast)
    : Stmt(ast)
{

}

} // namespace skr::SSL