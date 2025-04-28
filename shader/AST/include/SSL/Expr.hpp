#pragma once
#include "Enums.hpp"
#include "Decl.hpp"
#include "Stmt.hpp"

namespace skr::SSL {

struct BinaryExpr;
struct ParameterExpr;
struct ConstantExpr;
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

    Name dump() const override;

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
    Name dump() const override;
    const Decl* decl() const { return _decl; }

private:
    friend struct AST;
    DeclRefExpr(const AST& ast, const Decl& decl);
    const Decl* _decl = nullptr;
};

struct ConstantExpr : Expr
{
public:
    Name dump() const override;
    const String v;

private:
    friend struct AST;
    ConstantExpr(const AST& ast, const String& v);
};

}