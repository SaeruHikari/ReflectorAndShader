#include "SSL/Stmt.hpp"
#include "SSL/AST.hpp"

namespace skr::SSL {

Stmt::Stmt(AST& ast) : _ast(&ast) {}

void Stmt::add_child(const Stmt* child) 
{
    const_cast<Stmt*&>(child->_parent) = this;
    _children.emplace_back(child);
}

DeclStmt::DeclStmt(AST& ast, Decl* decl) : Stmt(ast), _decl(decl) {}

DeclRefExpr* DeclStmt::ref() const
{
    return const_cast<AST*>(_ast)->Ref(decl());
}

CompoundStmt::CompoundStmt(AST& ast, std::span<Stmt* const> statements) 
    : Stmt(ast)
{
    for (auto& statement : statements)
    {
        add_child(statement);
    }
}

void CompoundStmt::add_statement(Stmt* statement)
{
    add_child(statement);
}

IfStmt::IfStmt(AST& ast, Stmt* cond, CompoundStmt* then_body, CompoundStmt* else_body)
    : Stmt(ast), _cond(cond), _then_body(then_body), _else_body(else_body)
{
    add_child(cond);
    add_child(then_body);
    if (else_body) {
        add_child(else_body);
    }
}

ForStmt::ForStmt(AST& ast, Stmt* init, Stmt* cond, Stmt* inc, CompoundStmt* body)
    : Stmt(ast), _init(init), _cond(cond), _inc(inc), _body(body)
{
    add_child(init);
    add_child(cond);
    add_child(inc);
    add_child(body);
}

WhileStmt::WhileStmt(AST& ast, Stmt* cond, CompoundStmt* body)
    : Stmt(ast), _cond(cond), _body(body)
{
    add_child(cond);
    add_child(body);
}

BreakStmt::BreakStmt(AST& ast)
    : Stmt(ast)
{

}

ContinueStmt::ContinueStmt(AST& ast)
    : Stmt(ast)
{

}

DefaultStmt::DefaultStmt(AST& ast, CompoundStmt* body)
    : Stmt(ast)
{
    add_child(body);
}

SwitchStmt::SwitchStmt(AST& ast, Stmt* cond, std::span<CaseStmt*> cases)
    : Stmt(ast), _cond(cond), _cases(cases.begin(), cases.end())
{
    add_child(cond);
    for (auto case_stmt : _cases)
    {
        add_child(case_stmt);
    }
}

CaseStmt::CaseStmt(AST& ast, Stmt* cond, Stmt* body)
    : Stmt(ast), _cond(cond), _body(body)
{
    add_child(cond);
    add_child(body);
}

ReturnStmt::ReturnStmt(AST& ast, Stmt* value)
    : Stmt(ast), _value(value)
{
    add_child(value);
}

ValueStmt::ValueStmt(AST& ast)
    : Stmt(ast)
{

}

} // namespace skr::SSL