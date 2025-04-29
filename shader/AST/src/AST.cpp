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

TypeDecl* const AST::DeclareType(const Name& name, std::span<FieldDecl*> fields)
{
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return nullptr;
    auto new_type = new TypeDecl(*this, name, fields, false);
    _types.emplace_back(new_type);
    return new_type;
}

TypeDecl* const AST::DeclarePrimitiveType(const Name& name, uint32_t size, uint32_t alignment)
{
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return nullptr;
    auto new_type = new TypeDecl(*this, name, size, alignment, true);
    _types.emplace_back(new_type);
    return new_type;
}

ArrayTypeDecl* const AST::DeclareArrayType(TypeDecl* const element, uint32_t count)
{
    auto found = _arrs.find({element, count});
    if (found != _arrs.end())
        return found->second;
    auto new_type = new ArrayTypeDecl(*this, element, count);
    _types.emplace_back(new_type);
    _arrs.insert({{element, count}, new_type});
    return new_type;
}

FieldDecl* AST::Field(const Name& name, const TypeDecl* type)
{
    auto decl = new FieldDecl(*this, name, type);
    _decls.emplace_back(decl);
    return decl;
}

FunctionDecl* AST::Function(const Name& name, TypeDecl* const return_type, std::span<ParamVarDecl* const> params, CompoundStmt* body)
{
    auto decl = new FunctionDecl(*this, name, return_type, params, body);
    _decls.emplace_back(decl);
    _funcs.emplace_back(decl);
    return decl;
}

ParamVarDecl* AST::Param(const TypeDecl* type, const Name& name)
{
    auto decl = new ParamVarDecl(*this, name, type);
    _decls.emplace_back(decl);
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
    auto decl_name = L"decl" + std::to_wstring(_decls.size());
    return Variable(type, decl_name, initializer); 
}

DeclStmt* AST::Variable(const TypeDecl* type, const Name& name, Expr* initializer) 
{  
    auto decl = new VarDecl(*this, type, name, initializer);
    _decls.emplace_back(decl);

    auto stmt = new DeclStmt(*this, decl);
    _stmts.emplace_back(stmt);

    return stmt;
}

const TypeDecl* AST::GetType(const Name& name) const
{
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return *found;
    throw std::logic_error("Type not found");
}

#define USTR(x) L ## #x
#define INIT_BUILTIN_TYPE(symbol, type) symbol##Type(DeclarePrimitiveType(USTR(type), sizeof(type), alignof(type)))

#define INIT_VEC_TYPES(symbol, type) \
    symbol##2Type(DeclarePrimitiveType(USTR(type##2), sizeof(vec<type, 2>), alignof(vec<type, 2>))), \
    symbol##3Type(DeclarePrimitiveType(USTR(type##3), sizeof(vec<type, 3>), alignof(vec<type, 3>))), \
    symbol##4Type(DeclarePrimitiveType(USTR(type##4), sizeof(vec<type, 4>), alignof(vec<type, 4>)))

#define INIT_MATRIX_TYPE(symbol, type) \
    symbol##1x1Type(DeclarePrimitiveType(USTR(type##1x1), sizeof(matrix<type, 1, 1>), alignof(matrix<type, 1, 1>))), \
    symbol##1x2Type(DeclarePrimitiveType(USTR(type##1x2), sizeof(matrix<type, 1, 2>), alignof(matrix<type, 1, 2>))), \
    symbol##1x3Type(DeclarePrimitiveType(USTR(type##1x3), sizeof(matrix<type, 1, 3>), alignof(matrix<type, 1, 3>))), \
    symbol##1x4Type(DeclarePrimitiveType(USTR(type##1x4), sizeof(matrix<type, 1, 4>), alignof(matrix<type, 1, 4>))), \
    \
    symbol##2x1Type(DeclarePrimitiveType(USTR(type##2x1), sizeof(matrix<type, 2, 1>), alignof(matrix<type, 2, 1>))), \
    symbol##2x2Type(DeclarePrimitiveType(USTR(type##2x2), sizeof(matrix<type, 2, 2>), alignof(matrix<type, 2, 2>))), \
    symbol##2x3Type(DeclarePrimitiveType(USTR(type##2x3), sizeof(matrix<type, 2, 3>), alignof(matrix<type, 2, 3>))), \
    symbol##2x4Type(DeclarePrimitiveType(USTR(type##2x4), sizeof(matrix<type, 2, 4>), alignof(matrix<type, 2, 4>))), \
    \
    symbol##3x1Type(DeclarePrimitiveType(USTR(type##3x1), sizeof(matrix<type, 3, 1>), alignof(matrix<type, 3, 1>))), \
    symbol##3x2Type(DeclarePrimitiveType(USTR(type##3x2), sizeof(matrix<type, 3, 2>), alignof(matrix<type, 3, 2>))), \
    symbol##3x3Type(DeclarePrimitiveType(USTR(type##3x3), sizeof(matrix<type, 3, 3>), alignof(matrix<type, 3, 3>))), \
    symbol##3x4Type(DeclarePrimitiveType(USTR(type##3x4), sizeof(matrix<type, 3, 4>), alignof(matrix<type, 3, 4>))), \
    \
    symbol##4x1Type(DeclarePrimitiveType(USTR(type##4x1), sizeof(matrix<type, 4, 1>), alignof(matrix<type, 4, 1>))), \
    symbol##4x2Type(DeclarePrimitiveType(USTR(type##4x2), sizeof(matrix<type, 4, 2>), alignof(matrix<type, 4, 2>))), \
    symbol##4x3Type(DeclarePrimitiveType(USTR(type##4x3), sizeof(matrix<type, 4, 3>), alignof(matrix<type, 4, 3>))), \
    symbol##4x4Type(DeclarePrimitiveType(USTR(type##4x4), sizeof(matrix<type, 4, 4>), alignof(matrix<type, 4, 4>)))

AST::AST() : 
    VoidType(DeclarePrimitiveType(L"void", 0, 0)),
    
    INIT_BUILTIN_TYPE(Float, float),
    INIT_VEC_TYPES(Float, float),
    INIT_MATRIX_TYPE(Float, float),

    INIT_BUILTIN_TYPE(Int, int32_t),
    INIT_VEC_TYPES(Int, float),
    INIT_MATRIX_TYPE(Int, float),

    INIT_BUILTIN_TYPE(UInt, uint32_t),
    INIT_VEC_TYPES(UInt, float),
    INIT_MATRIX_TYPE(UInt, float),

    INIT_BUILTIN_TYPE(I64, int64_t),
    INIT_BUILTIN_TYPE(U64, uint64_t)
{
    
}

#undef INIT_MATRIX_TYPE
#undef INIT_VEC_TYPES
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