#pragma once
#include "Stmt.hpp"

namespace skr::SSL {

struct AST;
struct Expr;
struct TypeDecl;

struct Decl
{
public:
    virtual String dump() const;
    virtual ~Decl() = default;
    virtual const Stmt* body() const = 0;
    virtual const DeclRefExpr* ref() const;

protected:
    Decl(const AST& ast);
    const AST* _ast = nullptr;
};

struct VarDecl : public Decl
{
public:
    const TypeDecl& type() const { return *_type; }
    String name() const { return _name; }
    Expr* initializer() const { return _initializer; }
    const Stmt* body() const override;
    
protected:
    friend struct AST;    
    VarDecl(const AST& ast, const TypeDecl* type, const Name& name, Expr* initializer = nullptr);
    const TypeDecl* _type = nullptr;
    Name _name = L"__INVALID_VAR__";
    Expr* _initializer = nullptr;
};

struct FieldDecl : public Decl
{
public:
    const TypeDecl& type() const { return *_type; }
    const Name& name() const;
    const Size size() const;
    const Size alignment() const;
    const Stmt* body() const override;

protected:
    friend struct AST;    
    FieldDecl(const AST& ast, const Name& _name, const TypeDecl* type);
    Name _name = L"__INVALID_MEMBER__";
    const TypeDecl* _type = nullptr;
};

struct TypeDecl : public Decl
{
public:
    const Name& name() const { return _name; }
    bool is_builtin() const { return _is_builtin; }
    const Size size() const  { return _size; }
    const Size alignment() const { return _alignment; }
    const auto& fields() const { return _fields; }
    const Stmt* body() const override;

protected:
    friend struct AST;
    TypeDecl(const AST& ast, const Name& name, uint32_t size, uint32_t alignment = 4, bool is_builtin = true);
    TypeDecl(const AST& ast, const Name& name, std::span<FieldDecl*> fields, bool is_builtin = false);

    Name _name = L"__INVALID_TYPE__";
    const bool _is_builtin = true;
    Size _size = 0;
    Size _alignment = 0;
    std::vector<FieldDecl*> _fields;
};

struct ArrayTypeDecl : public TypeDecl
{
    protected:
    friend struct AST;
    ArrayTypeDecl(const AST& ast, TypeDecl* const element, uint32_t count);
};

struct ParamVarDecl : public Decl
{
public:
    const TypeDecl& type() const { return *_type; }
    const Name& name() const;
    const Stmt* body() const override;

    protected:
    friend struct AST;    
    ParamVarDecl(const AST& ast, const Name& _name, const TypeDecl* type);
    Name _name = L"__INVALID_MEMBER__";
    const TypeDecl* _type = nullptr;
};

struct FunctionDecl : public Decl
{
public:
    const Name& name() const { return _name; }
    const TypeDecl* return_type() const { return _return_type; }
    const auto& parameters() const { return _parameters; }
    const Stmt* body() const override { return _body; }

protected:
    friend struct AST;
    FunctionDecl(const AST& ast, const Name& name, TypeDecl* const return_type, std::span<ParamVarDecl* const> params, const CompoundStmt* body);
    const Name _name = L"__INVALID_FUNC__";
    const CompoundStmt* _body = nullptr;
    TypeDecl* const _return_type = nullptr;
    std::vector<const ParamVarDecl*> _parameters;
};

struct MethodDecl : public FunctionDecl
{
public:

protected:
    friend struct AST;
    MethodDecl(const AST& ast, const Name& name, TypeDecl* const return_type, std::span<ParamVarDecl* const> params, const CompoundStmt* body);
};

} // namespace skr::SSL