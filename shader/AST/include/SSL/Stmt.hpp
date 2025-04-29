#pragma once
#include <vector>
#include <span>
#include <string>

namespace skr::SSL {

using Size = uint32_t;
using String = std::wstring;
using Name = String;

struct AST;
struct Decl;
struct DeclRefExpr;

struct Stmt
{
public:
    virtual ~Stmt() = default;
    
    inline std::span<Stmt* const> children() const { return _children; }
    virtual String dump() const = 0;

protected:
    Stmt(const AST& ast);
    const AST* _ast = nullptr;
    std::vector<Stmt*> _children;
};

struct DeclStmt : Stmt
{
public:
    DeclRefExpr* ref() const;
    const Decl* decl() const { return _decl; }
    String dump() const override { return L"DeclStmt"; }

protected:
    friend struct AST;
    DeclStmt(const AST& ast, Decl* decl);
    // TODO: DeclGroup
    Decl* _decl = nullptr;
};

struct CompoundStmt final : Stmt
{
public:
    String dump() const override;

protected:
    friend struct AST;
    CompoundStmt(const AST& ast, std::span<Stmt* const> statements);
};

struct ValueStmt : public Stmt
{
    virtual ~ValueStmt() = default;
protected:
    ValueStmt(const AST& ast);
};

} // namespace skr::SSL