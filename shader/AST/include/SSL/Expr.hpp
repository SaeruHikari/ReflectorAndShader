#pragma once
#include "Enums.hpp"
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

struct DeclRefExpr : Expr
{
public:
    const Decl* decl() const { return _decl; }

private:
    friend struct AST;
    DeclRefExpr(const AST& ast, const Decl& decl);
    const Decl* _decl = nullptr;
};

struct CallExpr : Expr
{
public:
};

struct ConstantExpr : Expr
{
public:
    const String v;

private:
    friend struct AST;
    ConstantExpr(const AST& ast, const String& v);
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

private:
    friend struct AST;
    MemberExpr(const AST& ast, const DeclRefExpr* owner, const FieldDecl* field);
    const DeclRefExpr* _owner = nullptr;
    const Decl* _member_decl = nullptr;
};

} // namespace skr::SSL