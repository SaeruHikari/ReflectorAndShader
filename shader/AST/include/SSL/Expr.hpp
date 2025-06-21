#pragma once
#include <variant>
#include "Enums.hpp"
#include "Constant.hpp"
#include "Decl.hpp"
#include "Stmt.hpp"

namespace skr::SSL {

struct BinaryExpr;
struct ParameterExpr;
struct ConstantExpr;
struct InitListExpr;
using FloatSemantics = String;

struct Expr : public ValueStmt
{
public:
    virtual ~Expr() = default;

protected:
    Expr(AST& ast);
};

struct CastExpr : Expr
{
public:
    const TypeDecl* type() const { return _type; }
    Expr* expr() const { return _expr; }

protected:
    CastExpr(AST& ast, const TypeDecl* type, Expr* expr);
    const TypeDecl* _type = nullptr;
    Expr* _expr = nullptr;
};

struct BitwiseCastExpr : CastExpr
{
private:
    friend struct AST;
    BitwiseCastExpr(AST& ast, const TypeDecl* type, Expr* expr);
};

struct BinaryExpr : Expr
{
public:
    const Expr* left() const { return _left; }
    const Expr* right() const { return _right; }
    const BinaryOp op() const { return _op; }

private:
    friend struct AST;
    BinaryExpr(AST& ast, Expr* left, Expr* right, BinaryOp op);
    Expr* _left = nullptr;
    Expr* _right = nullptr;
    const BinaryOp _op = BinaryOp::ADD;
};

struct CallExpr : Expr
{
public:
    const DeclRefExpr* callee() const { return _callee; }
    std::span<Expr* const> args() const { return _args; }
private:
    friend struct AST;
    CallExpr(AST& ast, DeclRefExpr* callee, std::span<Expr*> args);
    const DeclRefExpr* _callee = nullptr;
    std::vector<Expr*> _args;
};

struct ConstantExpr : Expr
{
public:
    const std::variant<IntValue, FloatValue> value;

private:
    friend struct AST;
    ConstantExpr(AST& ast, const IntValue& v);
    ConstantExpr(AST& ast, const FloatValue& v);
};

struct ConstructExpr : Expr
{
public:
    const TypeDecl* type() const { return _type; }
    std::span<Expr* const> args() const { return _args; }

private:
    friend struct AST;
    ConstructExpr(AST& ast, const TypeDecl* type, std::span<Expr*> args);
    const TypeDecl* _type = nullptr;
    std::vector<Expr*> _args;
};

struct DeclRefExpr : Expr
{
public:
    const Decl* decl() const { return _decl; }

private:
    friend struct AST;
    DeclRefExpr(AST& ast, const Decl& decl);
    const Decl* _decl = nullptr;
};

struct ImplicitCastExpr : CastExpr
{
private:
    friend struct AST;
    ImplicitCastExpr(AST& ast, const TypeDecl* type, Expr* expr);
};

struct InitListExpr : Expr
{
private:
    friend struct AST;
    InitListExpr(AST& ast, std::span<Expr*> exprs);
    std::vector<Expr*> _exprs;
};

struct MemberExpr : Expr
{
public:
    const DeclRefExpr* owner() const { return _owner; }
    const Decl* member_decl() const { return _member_decl; }

protected:
    MemberExpr(AST& ast, const DeclRefExpr* owner, const Decl* field);
    const DeclRefExpr* _owner = nullptr;
    const Decl* _member_decl = nullptr;
};

struct FieldExpr : MemberExpr
{
public:
    const FieldDecl* field_decl() const;

private:
    friend struct AST;
    FieldExpr(AST& ast, const DeclRefExpr* owner, const FieldDecl* field);
};

struct MethodExpr : MemberExpr
{
public:
    const MethodDecl* method_decl() const;
    
private:
    friend struct AST;
    MethodExpr(AST& ast, const DeclRefExpr* owner, const FunctionDecl* method);
};

struct MethodCallExpr : Expr
{
public:
    const MemberExpr* callee() const { return _callee; }
    std::span<Expr* const> args() const { return _args; }

private:
    friend struct AST;
    MethodCallExpr(AST& ast, const MemberExpr* callee, std::span<Expr*> args);
    const MemberExpr* _callee = nullptr;
    std::vector<Expr*> _args;
};

struct SwizzleExpr : Expr
{
public:
    const Expr* expr() const { return _expr; }
    std::span<const uint64_t> seq() const { return std::span<const uint64_t>(_seq, _comps); }

private:
    friend struct AST;
    SwizzleExpr(AST& ast, Expr* expr, uint64_t comps, const uint64_t* seq);
    uint64_t _seq[4];
    uint64_t _comps = 0u;
    Expr* _expr = nullptr;
};

struct StaticCastExpr : CastExpr
{
private:
    friend struct AST;
    StaticCastExpr(AST& ast, const TypeDecl* type, Expr* expr);
};

struct ThisExpr final : Expr
{
private:
    friend struct AST;
    ThisExpr(AST& ast);
};

struct UnaryExpr : Expr
{
public:
    const UnaryOp op() const { return _op; }
    const Expr* expr() const { return _expr; }
private:
    friend struct AST;
    UnaryExpr(AST& ast, UnaryOp op, Expr* expr);
    const UnaryOp _op = UnaryOp::PLUS;
    Expr* _expr = nullptr;
};

} // namespace skr::SSL