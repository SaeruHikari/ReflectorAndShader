#pragma once
#include "Stmt.hpp"

namespace skr::SSL {

struct AST;
struct Attr;
struct Expr;
struct TypeDecl;
struct FieldDecl;
struct MethodDecl;
struct ConstructorDecl;
struct ConstantExpr;

struct Decl
{
public:
    virtual String dump() const;
    virtual ~Decl() = default;
    virtual const Stmt* body() const = 0;
    virtual DeclRefExpr* ref() const;

    std::span<Attr* const> attrs() const { return _attrs; }
    void add_attr(Attr* attr);

protected:
    Decl(AST& ast);
    const AST* _ast = nullptr;
    std::vector<Attr*> _attrs;
};

struct VarDecl : public Decl
{
public:
    const TypeDecl& type() const { return *_type; }
    String name() const { return _name; }
    Expr* initializer() const { return _initializer; }
    EVariableQualifier qualifier() const { return _qualifier; }
    const Stmt* body() const override;
    
protected:
    friend struct AST;    
    VarDecl(AST& ast, EVariableQualifier qualifier, const TypeDecl* type, const Name& name, Expr* initializer = nullptr);
    EVariableQualifier _qualifier = EVariableQualifier::None; // the qualifier of the variable (e.g., const, in, out, etc.)
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
    FieldDecl(AST& ast, const Name& _name, const TypeDecl* type);
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
    const auto& methods() const { return _methods; }
    const auto& ctors() const { return _ctors; }
    const Stmt* body() const override;

    void add_field(FieldDecl* field);
    void add_method(MethodDecl* method);
    void add_ctor(ConstructorDecl* ctor);

    FieldDecl* get_field(const Name& name) const;
    MethodDecl* get_method(const Name& name) const;

protected:
    friend struct AST;
    TypeDecl(AST& ast, const Name& name, uint32_t size, uint32_t alignment = 4, std::span<FieldDecl*> fields = {}, bool is_builtin = true);
    TypeDecl(AST& ast, const Name& name, std::span<FieldDecl*> fields, bool is_builtin = false);

    Name _name = L"__INVALID_TYPE__";
    const bool _is_builtin = true;
    Size _size = 0;
    Size _alignment = 0;
    std::vector<FieldDecl*> _fields;
    std::vector<MethodDecl*> _methods;
    std::vector<ConstructorDecl*> _ctors;
};

struct ResourceTypeDecl : public TypeDecl
{
protected:
    ResourceTypeDecl(AST& ast, const String& name);
};

struct BufferTypeDecl : public ResourceTypeDecl
{
public:
    const auto flags() const { return _flags; }

protected:
    BufferTypeDecl(AST& ast, const String& name, BufferFlags flags);
    BufferFlags _flags;
};

struct ByteBufferTypeDecl : public BufferTypeDecl
{
protected:
    friend struct AST;
    ByteBufferTypeDecl(AST& ast, BufferFlags flags);
};

struct StructuredBufferTypeDecl : public BufferTypeDecl
{
public:
    const TypeDecl& element() const { return *_element; }
    const Size element_size() const { return _element->size(); }
    const Size element_alignment() const { return _element->alignment(); }

protected:
    friend struct AST;
    StructuredBufferTypeDecl(AST& ast, TypeDecl* const element, BufferFlags flags);
    TypeDecl* const _element = nullptr; // the type of elements in the buffer
};

struct ArrayTypeDecl : public TypeDecl
{
protected:
    friend struct AST;
    ArrayTypeDecl(AST& ast, TypeDecl* const element, uint32_t count);
};

struct GlobalVarDecl : public VarDecl
{
public:
    const TypeDecl& type() const { return *_type; }
    
protected:
    friend struct AST;
    GlobalVarDecl(AST& ast, EVariableQualifier qualifier, const TypeDecl* type, const Name& _name, ConstantExpr* initializer);
};

struct ParamVarDecl : public VarDecl
{
public:
    const TypeDecl& type() const { return *_type; }

protected:
    friend struct AST;    
    ParamVarDecl(AST& ast, EVariableQualifier qualifier, const TypeDecl* type, const Name& _name);
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
    FunctionDecl(AST& ast, const Name& name, TypeDecl* const return_type, std::span<ParamVarDecl* const> params, const CompoundStmt* body);
    const Name _name = L"__INVALID_FUNC__";
    const CompoundStmt* _body = nullptr;
    TypeDecl* const _return_type = nullptr;
    std::vector<const ParamVarDecl*> _parameters;
};

struct MethodDecl : public FunctionDecl
{
public:
    const TypeDecl* owner_type() const { return _owner; }

protected:
    friend struct AST;
    MethodDecl(AST& ast, TypeDecl* owner, const Name& name, TypeDecl* const return_type, std::span<ParamVarDecl* const> params, const CompoundStmt* body);
    TypeDecl* _owner = nullptr; // the type that owns this method
};

struct ConstructorDecl : public MethodDecl
{
protected:
    friend struct AST;
    ConstructorDecl(AST& ast, TypeDecl* owner, const Name& name, std::span<ParamVarDecl* const> params, const CompoundStmt* body);
};

} // namespace skr::SSL