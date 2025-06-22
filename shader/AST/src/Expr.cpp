#include "SSL/Expr.hpp"
#include "SSL/AST.hpp"
#include "SSL/magic_enum/magic_enum.hpp"

namespace skr::SSL {

Expr::Expr(AST& ast) : ValueStmt(ast) {}

BinaryExpr::BinaryExpr(AST& ast, Expr* left, Expr* right, BinaryOp op) 
    : Expr(ast), _left(left), _right(right), _op(op) 
{
    add_child(left);
    add_child(right);
}

CastExpr::CastExpr(AST& ast, const TypeDecl* type, Expr* expr) 
    : Expr(ast), _type(type), _expr(expr) 
{
    add_child(expr);
}

BitwiseCastExpr::BitwiseCastExpr(AST& ast, const TypeDecl* type, Expr* expr) 
    : CastExpr(ast, type, expr)
{

}

CallExpr::CallExpr(AST& ast, DeclRefExpr* callee, std::span<Expr*> args) 
    : Expr(ast), _callee(callee), _args(args.begin(), args.end()) 
{
    add_child(callee);
    for (auto arg : _args)
        add_child(arg);
}

ConstantExpr::ConstantExpr(AST& ast, const IntValue& v) 
    : Expr(ast), value(v) 
{

}

ConstantExpr::ConstantExpr(AST& ast, const FloatValue& v) 
    : Expr(ast), value(v)
{
    
}

ConstructExpr::ConstructExpr(AST& ast, const TypeDecl* type, std::span<Expr*> args)
    : Expr(ast), _type(type), _args(args.begin(), args.end())
{
    for (auto arg : _args)
        add_child(arg);
}

DeclRefExpr::DeclRefExpr(AST& ast, const Decl& decl) 
    : Expr(ast), _decl(&decl) 
{

}

ImplicitCastExpr::ImplicitCastExpr(AST& ast, const TypeDecl* type, Expr* expr) 
    : CastExpr(ast, type, expr) 
{

}

InitListExpr::InitListExpr(AST& ast, std::span<Expr*> exprs) 
    : Expr(ast), _exprs(exprs.begin(), exprs.end()) 
{
    for (auto expr : _exprs)
        add_child(expr);
}

MemberExpr::MemberExpr(AST& ast, const Expr* owner, const Decl* field)
    : Expr(ast), _owner(owner), _member_decl(field)
{
    add_child(_owner);
}

FieldExpr::FieldExpr(AST& ast, const Expr* owner, const FieldDecl* field)
    : MemberExpr(ast, owner, field)
{

}

const FieldDecl* FieldExpr::field_decl() const
{
    return dynamic_cast<const FieldDecl*>(_member_decl);
}

MethodExpr::MethodExpr(AST& ast, const DeclRefExpr* owner, const FunctionDecl* method)
    : MemberExpr(ast, owner, method)
{

}

const MethodDecl* MethodExpr::method_decl() const
{
    return dynamic_cast<const MethodDecl*>(_member_decl);
}

MethodCallExpr::MethodCallExpr(AST& ast, const MemberExpr* callee, std::span<Expr*> args)
    : Expr(ast), _callee(callee), _args(args.begin(), args.end())
{
    add_child(_callee);
    for (auto arg : _args)
        add_child(arg);
}

SwizzleExpr::SwizzleExpr(AST& ast, Expr* expr, uint64_t comps, const uint64_t* seq)
    : Expr(ast), _expr(expr), _comps(comps)
{
    for (uint32_t i = 0; i < comps; i++)
        _seq[i] = seq[i];
    add_child(_expr);
}

StaticCastExpr::StaticCastExpr(AST& ast, const TypeDecl* type, Expr* expr)
    : CastExpr(ast, type, expr)
{

}

ThisExpr::ThisExpr(AST& ast)
    : Expr(ast)
{
    
}

UnaryExpr::UnaryExpr(AST& ast, UnaryOp op, Expr* expr)
    : Expr(ast), _op(op), _expr(expr)
{
    add_child(expr);
}

} // namespace skr::SSL