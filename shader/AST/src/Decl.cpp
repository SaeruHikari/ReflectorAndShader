#include "SSL/AST.hpp"
#include "SSL/Decl.hpp"

namespace skr::SSL {

Decl::Decl(const AST& ast) 
    : _ast(&ast) 
{

}

String VarDecl::dump() const
{
    return _name;
}

VarDecl::VarDecl(const AST& ast, const TypeDecl* type, const Name& name, Expr* initializer)
    : Decl(ast), _type(type), _name(name), _initializer(initializer)
{

}

FieldDecl::FieldDecl(const AST& ast, const Name& name, const TypeDecl& type)
    : Decl(ast), _name(name), _type(&type)
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

String FieldDecl::dump() const
{
    return u8"UNDEFINED";
}

TypeDecl::TypeDecl(const AST& ast, const Name& name, uint32_t size, uint32_t alignment, bool is_builtin)
    : Decl(ast), _name(name), _is_builtin(is_builtin), _size(size), _alignment(alignment)
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

String TypeDecl::dump() const
{
    return u8"UNDEFINED";
}

FunctionDecl::FunctionDecl(const AST& ast, const Name& name, const CompoundStmt* body)
    : Decl(ast), _name(name), _return_type(ast.VoidType), _body(body)
{
    _parameters.reserve(_parameters.size());
    for (const auto& param : _parameters)
    {
        _parameters.emplace_back(param);
    }
}

String FunctionDecl::dump() const
{
    return u8"UNDEFINED";
}

}