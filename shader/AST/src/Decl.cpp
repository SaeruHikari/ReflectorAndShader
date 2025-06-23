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

NamedDecl::NamedDecl(AST& ast, const Name& name)
    : Decl(ast), _name(name)
{

}

const Stmt* VarDecl::body() const
{
    return nullptr;
}

VarDecl::VarDecl(AST& ast, EVariableQualifier qualifier, const TypeDecl* type, const Name& name, Expr* initializer)
    : NamedDecl(ast, name), _qualifier(qualifier), _type(type), _initializer(initializer)
{

}

FieldDecl::FieldDecl(AST& ast, const Name& name, const TypeDecl* type)
    : NamedDecl(ast, name), _type(type)
{

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
    : NamedDecl(ast, name), _is_builtin(is_builtin), _size(size), _alignment(alignment), _fields(fields.begin(), fields.end())
{

}

TypeDecl::TypeDecl(AST& ast, const Name& name, std::span<FieldDecl*> fields, bool is_builtin)
    : NamedDecl(ast, name), _is_builtin(is_builtin)
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
    assert(method != nullptr && "Method cannot be null");
    assert(dynamic_cast<ConstructorDecl*>(method) == nullptr && "Cannot add constructor as method");
    
    _methods.emplace_back(method);
}

void TypeDecl::add_ctor(ConstructorDecl* ctor)
{
    assert(ctor != nullptr && "Constructor cannot be null");
    _ctors.emplace_back(ctor);
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
    /*
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
    */
}

StructuredBufferTypeDecl::StructuredBufferTypeDecl(AST& ast, const TypeDecl* element, BufferFlags flags)
    : BufferTypeDecl(ast, std::format(L"{}StructuredBuffer<{}>", (flags & (uint32_t)BufferFlag::ReadWrite) ? L"RW" : L"", element->name()), flags), _element(element)
{

}

TextureTypeDecl::TextureTypeDecl(AST& ast, const String& name, const TypeDecl* element, TextureFlags flags)
    : ResourceTypeDecl(ast, name), _element(element), _flags(flags)
{

}

Texture2DTypeDecl::Texture2DTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags)
    : TextureTypeDecl(ast, std::format(L"{}Texture2D<{}>", (flags & (uint32_t)TextureFlag::ReadWrite) ? L"RW" : L"", element->name()), element, flags)
{

}

Texture3DTypeDecl::Texture3DTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags)
    : TextureTypeDecl(ast, std::format(L"{}Texture3D<{}>", (flags & (uint32_t)TextureFlag::ReadWrite) ? L"RW" : L"", element->name()), element, flags)
{

}

ArrayTypeDecl::ArrayTypeDecl(AST& ast, const TypeDecl* element, uint32_t count)
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

FunctionDecl::FunctionDecl(AST& ast, const Name& name, const TypeDecl* return_type, std::span<const ParamVarDecl* const> params, const CompoundStmt* body)
    : NamedDecl(ast, name), _return_type(return_type), _body(body)
{
    _parameters.reserve(_parameters.size());
    for (const auto& param : params)
    {
        _parameters.emplace_back(param);
    }
}

const TypeDecl* FunctionDecl::return_type() const 
{ 
    return _return_type; 
}

const std::span<const ParamVarDecl* const> FunctionDecl::parameters() const
{
    return std::span<const ParamVarDecl* const>(_parameters);
}

MethodDecl::MethodDecl(AST& ast, TypeDecl* owner, const Name& name, const TypeDecl* return_type, std::span<const ParamVarDecl* const> params, const CompoundStmt* body)
    : FunctionDecl(ast, name, return_type, params, body), _owner(owner)
{

}

ConstructorDecl::ConstructorDecl(AST& ast, TypeDecl* owner, const Name& name, std::span<const ParamVarDecl* const> params, const CompoundStmt* body)
    : MethodDecl(ast, owner, name, ast.VoidType, params, body)
{

}

VarConceptDecl::VarConceptDecl(AST& ast, const Name& name)
    : NamedDecl(ast, name)
{

}

const Stmt* VarConceptDecl::body() const
{
    return nullptr; // Concepts have no body
}

TemplateCallableDecl::TemplateCallableDecl(AST& ast, const Name& name, std::span<const VarConceptDecl* const> param_concepts)
    : NamedDecl(ast, name), _owner(nullptr)
{
    _parameter_concepts.reserve(param_concepts.size());
    for (auto param_concept : param_concepts) {
        _parameter_concepts.push_back(param_concept);
    }
}

TemplateCallableDecl::TemplateCallableDecl(AST& ast, TypeDecl* owner, const Name& name, std::span<const VarConceptDecl* const> param_concepts)
    : NamedDecl(ast, name), _owner(owner)
{
    _parameter_concepts.reserve(param_concepts.size());
    for (auto param_concept : param_concepts) {
        _parameter_concepts.push_back(param_concept);
    }
}

bool TemplateCallableDecl::can_call_with(std::span<const TypeDecl* const> arg_types, 
                                        std::span<const EVariableQualifier> arg_qualifiers) const
{
    if (arg_types.size() != _parameter_concepts.size() || 
        arg_qualifiers.size() != _parameter_concepts.size()) {
        return false;
    }
      for (size_t i = 0; i < arg_types.size(); ++i) {
        const auto& param_concept = _parameter_concepts[i];
        // Check both type and qualifier constraints in validate method
        if (!param_concept->validate(arg_qualifiers[i], arg_types[i])) {
            return false;
        }
    }
    
    return true;
}

FunctionDecl* TemplateCallableDecl::specialize_for(std::span<const TypeDecl* const> arg_types, 
                                                  std::span<const EVariableQualifier> arg_qualifiers) const
{
    if (!can_call_with(arg_types, arg_qualifiers)) {
        return nullptr;
    }
    
    // Create specialized function or method based on whether this template has an owner
    if (is_method()) {
        return new SpecializedMethodDecl(const_cast<AST&>(ast()), this, arg_types, arg_qualifiers);
    } else {
        return new SpecializedFunctionDecl(const_cast<AST&>(ast()), this, arg_types, arg_qualifiers);
    }
}

// SpecializedFunctionDecl implementation
SpecializedFunctionDecl::SpecializedFunctionDecl(AST& ast, const TemplateCallableDecl* template_decl, 
                                                 std::span<const TypeDecl* const> arg_types,
                                                 std::span<const EVariableQualifier> arg_qualifiers)
    : FunctionDecl(ast, template_decl->name(), template_decl->get_return_type_for(arg_types), {}, nullptr), 
      _template(template_decl)
{
    // Create concrete parameters from template concepts and argument types
    _parameters.reserve(arg_types.size());
    auto concepts = template_decl->parameter_concepts();
    
    for (size_t i = 0; i < arg_types.size(); ++i) {
        auto param_name = concepts[i]->name(); // Use concept name as parameter name
        auto param = ast.DeclareParam(arg_qualifiers[i], arg_types[i], param_name);
        _parameters.push_back(param);
    }
}

// SpecializedMethodDecl implementation
SpecializedMethodDecl::SpecializedMethodDecl(AST& ast, const TemplateCallableDecl* template_decl, 
                                             std::span<const TypeDecl* const> arg_types,
                                             std::span<const EVariableQualifier> arg_qualifiers)
    : MethodDecl(ast, const_cast<TypeDecl*>(template_decl->owner_type()), template_decl->name(), 
                 template_decl->get_return_type_for(arg_types), {}, nullptr), 
      _template(template_decl)
{
    // Create concrete parameters from template concepts and argument types
    _parameters.reserve(arg_types.size());
    auto concepts = template_decl->parameter_concepts();
    
    for (size_t i = 0; i < arg_types.size(); ++i) {
        auto param_name = concepts[i]->name(); // Use concept name as parameter name
        auto param = ast.DeclareParam(arg_qualifiers[i], arg_types[i], param_name);
        _parameters.push_back(param);
    }
}

}