#include "SSL/Expr.hpp"
#include "SSL/AST.hpp"
#include "magic_enum/magic_enum.hpp"

namespace skr::SSL {

Expr::Expr(const AST& ast) : ValueStmt(ast) {}

BinaryExpr::BinaryExpr(const AST& ast, Expr* left, Expr* right, BinaryOp op) 
    : Expr(ast), _left(left), _right(right), _op(op) 
{
    _children.emplace_back(left);
    _children.emplace_back(right);
}

CastExpr::CastExpr(const AST& ast, const TypeDecl* type, Expr* expr) 
    : Expr(ast), _type(type), _expr(expr) 
{
    _children.emplace_back(expr);
}

BitwiseCastExpr::BitwiseCastExpr(const AST& ast, const TypeDecl* type, Expr* expr) 
    : CastExpr(ast, type, expr)
{

}

CallExpr::CallExpr(const AST& ast, DeclRefExpr* callee, std::span<Expr*> args) 
    : Expr(ast), _callee(callee), _args(args.begin(), args.end()) 
{
    _children.emplace_back(callee);
    for (auto arg : _args)
        _children.emplace_back(arg);
}

ConstantExpr::ConstantExpr(const AST& ast, const IntValue& v) 
    : Expr(ast), value(v) 
{

}

ConstantExpr::ConstantExpr(const AST& ast, const FloatValue& v) 
    : Expr(ast), value(v)
{
    
}

ConstructExpr::ConstructExpr(const AST& ast, const TypeDecl* type, std::span<Expr*> args)
    : Expr(ast), _type(type), _args(args.begin(), args.end())
{
    for (auto arg : _args)
        _children.emplace_back(arg);
}

DeclRefExpr::DeclRefExpr(const AST& ast, const Decl& decl) 
    : Expr(ast), _decl(&decl) 
{

}

ImplicitCastExpr::ImplicitCastExpr(const AST& ast, const TypeDecl* type, Expr* expr) 
    : CastExpr(ast, type, expr) 
{

}

InitListExpr::InitListExpr(const AST& ast, std::span<Expr*> exprs) 
    : Expr(ast), _exprs(exprs.begin(), exprs.end()) 
{
    for (auto expr : _exprs)
        _children.emplace_back(expr);
}

MemberExpr::MemberExpr(const AST& ast, const DeclRefExpr* owner, const Decl* field)
    : Expr(ast), _owner(owner), _member_decl(field)
{
    _children.emplace_back(_owner);
}

FieldExpr::FieldExpr(const AST& ast, const DeclRefExpr* owner, const FieldDecl* field)
    : MemberExpr(ast, owner, field)
{

}

MethodExpr::MethodExpr(const AST& ast, const DeclRefExpr* owner, const FunctionDecl* method)
    : MemberExpr(ast, owner, method)
{

}

MethodCallExpr::MethodCallExpr(const AST& ast, const MemberExpr* callee, std::span<Expr*> args)
    : Expr(ast), _callee(callee), _args(args.begin(), args.end())
{
    _children.emplace_back(_callee);
    for (auto arg : _args)
        _children.emplace_back(arg);
}

StaticCastExpr::StaticCastExpr(const AST& ast, const TypeDecl* type, Expr* expr)
    : Expr(ast)
{

}

UnaryExpr::UnaryExpr(const AST& ast, UnaryOp op, Expr* expr)
    : Expr(ast), _op(op), _expr(expr)
{
    _children.emplace_back(expr);
}

} // namespace skr::SSL