#include "SSL/AST.hpp"
#include <stdexcept>

namespace skr::SSL 
{

BinaryExpr* AST::Add(Expr* left, Expr* right)
{
    auto expr = new BinaryExpr(*this, left, right, BinaryOp::ADD);
    _stmts.emplace_back(expr);
    return expr;
}
    
BinaryExpr* AST::Assign(Expr* left, Expr* right)
{
    auto expr = new BinaryExpr(*this, left, right, BinaryOp::ASSIGN);
    _stmts.emplace_back(expr);
    return expr;
}

ConstantExpr* AST::Constant(const FloatSemantics& v) 
{ 
    auto expr = new ConstantExpr(*this, v); 
    _stmts.emplace_back(expr);
    return expr;
}

FieldDecl* AST::Field(const Name& name, const TypeDecl* type)
{
    auto decl = new FieldDecl(*this, name, *type);
    _decls.emplace_back(decl);
    return decl;
}

FunctionDecl* AST::Function(const Name& name, CompoundStmt* body)
{
    auto decl = new FunctionDecl(*this, name, body);
    _decls.emplace_back(decl);
    _funcs.emplace_back(decl);
    return decl;
}

DeclRefExpr* AST::Ref(const DeclStmt* decl)
{
    auto expr = new DeclRefExpr(*this, *decl->decl());
    _stmts.emplace_back(expr);
    return expr;
}

DeclStmt* AST::Variable(const TypeDecl* type, Expr* initializer) 
{  
    auto decl_name = "decl" + std::to_string(_decls.size());
    return Variable(type, (const char8_t*)decl_name.c_str(), initializer); 
}

DeclStmt* AST::Variable(const TypeDecl* type, const Name& name, Expr* initializer) 
{  
    auto decl = new VarDecl(*this, type, (const char8_t*)name.c_str(), initializer);
    _decls.emplace_back(decl);

    auto stmt = new DeclStmt(*this, decl);
    _stmts.emplace_back(stmt);

    return stmt;
}

TypeDecl* const AST::AddType(const Name& name, std::span<FieldDecl*> fields)
{
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return nullptr;
    auto new_type = new TypeDecl(*this, name, fields, false);
    _types.emplace_back(new_type);
    return new_type;
}

TypeDecl* const AST::AddBuiltinType(const Name& name, uint32_t size, uint32_t alignment)
{
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return nullptr;
    auto new_type = new TypeDecl(*this, name, size, alignment, true);
    _types.emplace_back(new_type);
    return new_type;
}

const TypeDecl* AST::GetType(const Name& name) const
{
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return *found;
    throw std::logic_error("Type not found");
}

#define U8STR(x) u8 ## x
#define INIT_BUILTIN_TYPE(symbol, type) symbol(AddBuiltinType(U8STR(#type), sizeof(type), alignof(type)))

AST::AST() : 
    VoidType(AddBuiltinType(u8"void", 0, 0)),
    INIT_BUILTIN_TYPE(F32Type, float),
    INIT_BUILTIN_TYPE(I32Type, int32_t),
    INIT_BUILTIN_TYPE(I64Type, int64_t),
    INIT_BUILTIN_TYPE(U32Type, uint32_t),
    INIT_BUILTIN_TYPE(U64Type, uint64_t)
{
    
}

#undef INIT_BUILTIN_TYPE
#undef U8STR

AST::~AST()
{
    for (auto stmt : _stmts)
    {
        delete stmt;
    }
    _stmts.clear();

    for (auto decl : _decls)
    {
        delete decl;
    }
    _decls.clear();
}

} // namespace skr::SSL