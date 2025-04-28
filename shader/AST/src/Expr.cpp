#include "SSL/Expr.hpp"
#include "SSL/AST.hpp"
#include "magic_enum/magic_enum.hpp"

namespace skr::SSL {

Expr::Expr(const AST& ast) : ValueStmt(ast) {}

Name BinaryExpr::dump() const
{
    auto name = magic_enum::enum_name(op());
    return Name((const char8_t*)name.data(), name.size());
}

BinaryExpr::BinaryExpr(const AST& ast, Expr* left, Expr* right, BinaryOp op) 
    : Expr(ast), _left(left), _right(right), _op(op) 
{
    _children.emplace_back(left);
    _children.emplace_back(right);
}

DeclRefExpr::DeclRefExpr(const AST& ast, const Decl& decl) 
    : Expr(ast), _decl(&decl) 
{

}

Name DeclRefExpr::dump() const
{
    return _decl->dump();
}

ConstantExpr::ConstantExpr(const AST& ast, const String& v) 
    : Expr(ast), v(v) 
{

}

Name ConstantExpr::dump() const
{
    return v;
}

} // namespace skr::SSL