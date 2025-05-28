#include "SSL/AST.hpp"
#include "SSL/Decl.hpp"
#include <format>

namespace skr::SSL {

Decl::Decl(const AST& ast) 
    : _ast(&ast) 
{

}

DeclRefExpr* Decl::ref() const
{
    return const_cast<AST*>(_ast)->Ref(this);
}

const Stmt* VarDecl::body() const
{
    return nullptr;
}

VarDecl::VarDecl(const AST& ast, const TypeDecl* type, const Name& name, Expr* initializer)
    : Decl(ast), _type(type), _name(name), _initializer(initializer)
{

}

FieldDecl::FieldDecl(const AST& ast, const Name& name, const TypeDecl* type)
    : Decl(ast), _name(name), _type(type)
{

}

const Name& FieldDecl::name() const
{
    return _name;
}

const Size FieldDecl::size() const
{
    return _type->size();
}

const Size FieldDecl::alignment() const
{
    return _type->alignment();
}

const Stmt* FieldDecl::body() const
{
    return nullptr;
}

TypeDecl::TypeDecl(const AST& ast, const Name& name, uint32_t size, uint32_t alignment, std::span<FieldDecl*> fields, bool is_builtin)
    : Decl(ast), _name(name), _is_builtin(is_builtin), _size(size), _alignment(alignment), _fields(fields.begin(), fields.end())
{

}

TypeDecl::TypeDecl(const AST& ast, const Name& name, std::span<FieldDecl*> fields, bool is_builtin)
    : Decl(ast), _name(name), _is_builtin(is_builtin)
{
    _fields.reserve(_fields.size());
    for (const auto& field : fields)
    {
        _fields.emplace_back(field);
        _size += field->size();
        _alignment = std::max(_alignment, field->alignment());
    }
}

const Stmt* TypeDecl::body() const
{
    return nullptr;
}

void TypeDecl::add_field(FieldDecl* field)
{
    _fields.emplace_back(field);
    _size += field->size();
    _alignment = std::max(_alignment, field->alignment());
}

void TypeDecl::add_method(MethodDecl* method)
{
    _methods.emplace_back(method);
}

FieldDecl* TypeDecl::get_field(const Name& name) const
{
    auto found = std::find_if(_fields.begin(), _fields.end(), [&](const FieldDecl* field) { return field->name() == name; });
    if (found != _fields.end())
        return *found;
    return nullptr;
}

MethodDecl* TypeDecl::get_method(const Name& name) const
{
    auto found = std::find_if(_methods.begin(), _methods.end(), [&](const MethodDecl* method) { return method->name() == name; });
    if (found != _methods.end())
        return *found;
    return nullptr;
}

ArrayTypeDecl::ArrayTypeDecl(const AST& ast, TypeDecl* const element, uint32_t count)
    : TypeDecl(ast, std::format(L"array<{}, {}>", element->name(), count), element->size() * count, element->alignment(), {}, true)
{

}

ParamVarDecl::ParamVarDecl(const AST& ast, const TypeDecl* type, const Name& name)
    : VarDecl(ast, type, name)
{

}

FunctionDecl::FunctionDecl(const AST& ast, const Name& name, TypeDecl* const return_type, std::span<ParamVarDecl* const> params, const CompoundStmt* body)
    : Decl(ast), _name(name), _return_type(return_type), _body(body)
{
    _parameters.reserve(_parameters.size());
    for (const auto& param : params)
    {
        _parameters.emplace_back(param);
    }
}

MethodDecl::MethodDecl(const AST& ast, TypeDecl* owner, const Name& name, TypeDecl* const return_type, std::span<ParamVarDecl* const> params, const CompoundStmt* body)
    : FunctionDecl(ast, name, return_type, params, body)
{

}

}