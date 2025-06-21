#include "SSL/AST.hpp"
#include "SSL/Decl.hpp"
#include <format>

namespace skr::SSL {

Decl::Decl(AST& ast) 
    : _ast(&ast) 
{

}

void Decl::add_attr(Attr* attr)
{
    if (attr)
    {
        _attrs.emplace_back(attr);
    }
}

DeclRefExpr* Decl::ref() const
{
    return const_cast<AST*>(_ast)->Ref(this);
}

const Stmt* VarDecl::body() const
{
    return nullptr;
}

VarDecl::VarDecl(AST& ast, EVariableQualifier qualifier, const TypeDecl* type, const Name& name, Expr* initializer)
    : Decl(ast), _qualifier(qualifier), _type(type), _name(name), _initializer(initializer)
{

}

FieldDecl::FieldDecl(AST& ast, const Name& name, const TypeDecl* type)
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

TypeDecl::TypeDecl(AST& ast, const Name& name, uint32_t size, uint32_t alignment, std::span<FieldDecl*> fields, bool is_builtin)
    : Decl(ast), _name(name), _is_builtin(is_builtin), _size(size), _alignment(alignment), _fields(fields.begin(), fields.end())
{

}

TypeDecl::TypeDecl(AST& ast, const Name& name, std::span<FieldDecl*> fields, bool is_builtin)
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

ResourceTypeDecl::ResourceTypeDecl(AST& ast, const String& name)
    : TypeDecl(ast, name, 0, 0, {}, true)
{

}

BufferTypeDecl::BufferTypeDecl(AST& ast, const String& name, BufferFlags flags)
    : ResourceTypeDecl(ast, name), _flags(flags)
{

}

ByteBufferTypeDecl::ByteBufferTypeDecl(AST& ast, BufferFlags flags)
    : BufferTypeDecl(ast, std::format(L"{}ByteAddressBuffer", (flags & (uint32_t)BufferFlag::ReadWrite) ? L"RW" : L""), flags)
{
    this->add_method(ast.DeclareMethod(this, L"GetCount", ast.UIntType, {}, nullptr));

    std::vector<ParamVarDecl*> address = { ast.DeclareParam(EVariableQualifier::None, ast.UIntType, L"address") };
    this->add_method(ast.DeclareMethod(this, L"Load", ast.UIntType, address, nullptr));
    this->add_method(ast.DeclareMethod(this, L"Load2", ast.UInt2Type, address, nullptr));
    this->add_method(ast.DeclareMethod(this, L"Load3", ast.UInt3Type, address, nullptr));
    this->add_method(ast.DeclareMethod(this, L"Load4", ast.UInt4Type, address, nullptr));

    if (flags & (uint32_t)BufferFlag::ReadWrite)
    {
        this->add_method(ast.DeclareMethod(this, L"Store", ast.VoidType, address, nullptr));
        this->add_method(ast.DeclareMethod(this, L"Store2", ast.VoidType, address, nullptr));
        this->add_method(ast.DeclareMethod(this, L"Store3", ast.VoidType, address, nullptr));
        this->add_method(ast.DeclareMethod(this, L"Store4", ast.VoidType, address, nullptr));

        std::vector<ParamVarDecl*> ps0 = {
            ast.DeclareParam(EVariableQualifier::None, ast.UIntType, L"dest"),
            ast.DeclareParam(EVariableQualifier::None, ast.UIntType, L"value")
        };
        this->add_method(ast.DeclareMethod(this, L"InterlockedAdd", ast.UIntType, ps0, nullptr));
        this->add_method(ast.DeclareMethod(this, L"InterlockedAnd", ast.UIntType, ps0, nullptr));
        this->add_method(ast.DeclareMethod(this, L"InterlockedMin", ast.UIntType, ps0, nullptr));
        this->add_method(ast.DeclareMethod(this, L"InterlockedMax", ast.UIntType, ps0, nullptr));
        this->add_method(ast.DeclareMethod(this, L"InterlockedOr", ast.UIntType, ps0, nullptr));
        this->add_method(ast.DeclareMethod(this, L"InterlockedXor", ast.UIntType, ps0, nullptr));
        this->add_method(ast.DeclareMethod(this, L"InterlockedExchange", ast.UIntType, ps0, nullptr));

        std::vector<ParamVarDecl*> ps1 = {
            ast.DeclareParam(EVariableQualifier::None, ast.UIntType, L"dest"),
            ast.DeclareParam(EVariableQualifier::None, ast.UIntType, L"compare"),
            ast.DeclareParam(EVariableQualifier::None, ast.UIntType, L"value")
        };
        this->add_method(ast.DeclareMethod(this, L"InterlockedCompareStore", ast.VoidType, ps1, nullptr));
        this->add_method(ast.DeclareMethod(this, L"InterlockedCompareExchange", ast.UIntType, ps1, nullptr));
    }

    // TODO: GENERIC LOAD/STORE
}

StructuredBufferTypeDecl::StructuredBufferTypeDecl(AST& ast, TypeDecl* const element, BufferFlags flags)
    : BufferTypeDecl(ast, std::format(L"{}StructuredBuffer<{}>", (flags & (uint32_t)BufferFlag::ReadWrite) ? L"RW" : L"", element->name()), flags), _element(element)
{
    this->add_method(ast.DeclareMethod(this, L"GetCount", element, {}, nullptr));
    
    std::vector<ParamVarDecl*> address = { ast.DeclareParam(EVariableQualifier::None, ast.UIntType, L"address") };
    this->add_method(ast.DeclareMethod(this, L"Load", element, address, nullptr));

    if (flags & (uint32_t)BufferFlag::ReadWrite)
    {
        std::vector<ParamVarDecl*> address_val = { 
            ast.DeclareParam(EVariableQualifier::None, ast.UIntType, L"address"),
            ast.DeclareParam(EVariableQualifier::None, element, L"value")
        };
        this->add_method(ast.DeclareMethod(this, L"Store", ast.VoidType, address_val, nullptr));
    }
}

ArrayTypeDecl::ArrayTypeDecl(AST& ast, TypeDecl* const element, uint32_t count)
    : TypeDecl(ast, std::format(L"array<{}, {}>", element->name(), count), element->size() * count, element->alignment(), {}, true)
{

}

GlobalVarDecl::GlobalVarDecl(AST& ast, EVariableQualifier qualifier, const TypeDecl* type, const Name& _name, ConstantExpr* initializer)
    : VarDecl(ast, qualifier, type, _name, initializer)
{

}

ParamVarDecl::ParamVarDecl(AST& ast, EVariableQualifier qualifier, const TypeDecl* type, const Name& name)
    : VarDecl(ast, qualifier, type, name)
{

}

FunctionDecl::FunctionDecl(AST& ast, const Name& name, TypeDecl* const return_type, std::span<ParamVarDecl* const> params, const CompoundStmt* body)
    : Decl(ast), _name(name), _return_type(return_type), _body(body)
{
    _parameters.reserve(_parameters.size());
    for (const auto& param : params)
    {
        _parameters.emplace_back(param);
    }
}

MethodDecl::MethodDecl(AST& ast, TypeDecl* owner, const Name& name, TypeDecl* const return_type, std::span<ParamVarDecl* const> params, const CompoundStmt* body)
    : FunctionDecl(ast, name, return_type, params, body), _owner(owner)
{

}

}