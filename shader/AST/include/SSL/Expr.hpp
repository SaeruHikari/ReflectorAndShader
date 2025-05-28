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
    Expr(const AST& ast);
};

struct CastExpr : Expr
{
public:
    const TypeDecl* type() const { return _type; }
    Expr* expr() const { return _expr; }

protected:
    CastExpr(const AST& ast, const TypeDecl* type, Expr* expr);
    const TypeDecl* _type = nullptr;
    Expr* _expr = nullptr;
};

struct BitwiseCastExpr : CastExpr
{
private:
    friend struct AST;
    BitwiseCastExpr(const AST& ast, const TypeDecl* type, Expr* expr);
};

struct BinaryExpr : Expr
{
public:
    const Expr* left() const { return _left; }
    const Expr* right() const { return _right; }
    const BinaryOp op() const { return _op; }

private:
    friend struct AST;
    BinaryExpr(const AST& ast, Expr* left, Expr* right, BinaryOp op);
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
    CallExpr(const AST& ast, DeclRefExpr* callee, std::span<Expr*> args);
    const DeclRefExpr* _callee = nullptr;
    std::vector<Expr*> _args;
};

struct ConstantExpr : Expr
{
public:
    const std::variant<IntValue, FloatValue> value;

private:
    friend struct AST;
    ConstantExpr(const AST& ast, const IntValue& v);
    ConstantExpr(const AST& ast, const FloatValue& v);
};

struct ConstructExpr : Expr
{
public:
    const TypeDecl* type() const { return _type; }
    std::span<Expr* const> args() const { return _args; }

private:
    friend struct AST;
    ConstructExpr(const AST& ast, const TypeDecl* type, std::span<Expr*> args);
    const TypeDecl* _type = nullptr;
    std::vector<Expr*> _args;
};

struct DeclRefExpr : Expr
{
public:
    const Decl* decl() const { return _decl; }

private:
    friend struct AST;
    DeclRefExpr(const AST& ast, const Decl& decl);
    const Decl* _decl = nullptr;
};

struct ImplicitCastExpr : CastExpr
{
private:
    friend struct AST;
    ImplicitCastExpr(const AST& ast, const TypeDecl* type, Expr* expr);
};

struct InitListExpr : Expr
{
private:
    friend struct AST;
    InitListExpr(const AST& ast, std::span<Expr*> exprs);
    std::vector<Expr*> _exprs;
};

struct MemberExpr : Expr
{
public:
    const DeclRefExpr* owner() const { return _owner; }
    const Decl* member_decl() const { return _member_decl; }

protected:
    MemberExpr(const AST& ast, const DeclRefExpr* owner, const Decl* field);
    const DeclRefExpr* _owner = nullptr;
    const Decl* _member_decl = nullptr;
};

struct FieldExpr : MemberExpr
{
public:
    const FieldDecl* field_decl() const { return dynamic_cast<const FieldDecl*>(_member_decl); }

private:
    friend struct AST;
    FieldExpr(const AST& ast, const DeclRefExpr* owner, const FieldDecl* field);
};

struct MethodExpr : MemberExpr
{
public:
    const FunctionDecl* method_decl() const { return dynamic_cast<const FunctionDecl*>(_member_decl); }
    
private:
    friend struct AST;
    MethodExpr(const AST& ast, const DeclRefExpr* owner, const FunctionDecl* method);
};

struct MethodCallExpr : Expr
{
public:
    const MemberExpr* callee() const { return _callee; }
    std::span<Expr* const> args() const { return _args; }

private:
    friend struct AST;
    MethodCallExpr(const AST& ast, const MemberExpr* callee, std::span<Expr*> args);
    const MemberExpr* _callee = nullptr;
    std::vector<Expr*> _args;
};

struct StaticCastExpr : Expr
{
private:
    friend struct AST;
    StaticCastExpr(const AST& ast, const TypeDecl* type, Expr* expr);
};

struct UnaryExpr : Expr
{
public:
    const UnaryOp op() const { return _op; }
    const Expr* expr() const { return _expr; }
private:
    friend struct AST;
    UnaryExpr(const AST& ast, UnaryOp op, Expr* expr);
    const UnaryOp _op = UnaryOp::PLUS;
    Expr* _expr = nullptr;
};

} // namespace skr::SSL