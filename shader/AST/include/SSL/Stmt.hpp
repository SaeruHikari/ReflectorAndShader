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
    
    inline std::span<const Stmt* const> children() const { return _children; }
    String dump() const;

protected:
    Stmt(const AST& ast);
    const AST* _ast = nullptr;
    std::vector<const Stmt*> _children;
};

struct DeclStmt : Stmt
{
public:
    DeclRefExpr* ref() const;
    const Decl* decl() const { return _decl; }

protected:
    friend struct AST;
    DeclStmt(const AST& ast, Decl* decl);
    // TODO: DeclGroup
    Decl* _decl = nullptr;
};

struct CompoundStmt final : Stmt
{
public:
    void add_statement(Stmt* statement);
    
protected:
    friend struct AST;
    CompoundStmt(const AST& ast, std::span<Stmt* const> statements);
};

struct ReturnStmt final : Stmt
{
public:
    const Stmt* value() const { return _value; }
protected:
    friend struct AST;
    ReturnStmt(const AST& ast, Stmt* value);
    Stmt* _value = nullptr;
};

struct ValueStmt : public Stmt
{
    virtual ~ValueStmt() = default;
protected:
    ValueStmt(const AST& ast);
};

} // namespace skr::SSL