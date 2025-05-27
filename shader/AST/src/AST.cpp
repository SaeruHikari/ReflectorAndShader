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

CompoundStmt* AST::Block(const std::vector<Stmt*>& statements)
{
    auto exp = new CompoundStmt(*this, statements);
    _stmts.emplace_back(exp);
    return exp;
}

CallExpr* AST::Call(DeclRefExpr* callee, std::span<Expr*> args)
{
    auto expr = new CallExpr(*this, callee, args);
    _stmts.emplace_back(expr);
    return expr;
}

ConstantExpr* AST::Constant(const IntValue& v) 
{ 
    auto expr = new ConstantExpr(*this, v); 
    _stmts.emplace_back(expr);
    return expr;
}

ConstantExpr* AST::Constant(const FloatValue& v) 
{ 
    auto expr = new ConstantExpr(*this, v); 
    _stmts.emplace_back(expr);
    return expr;
}

ConstructExpr* AST::Construct(const TypeDecl* type, std::span<Expr*> args)
{
    auto expr = new ConstructExpr(*this, type, args);
    _stmts.emplace_back(expr);
    return expr;
}

MemberExpr* AST::Member(DeclRefExpr* base, const FieldDecl* field)
{
    auto expr = new MemberExpr(*this, base, field);
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

TypeDecl* const AST::DeclarePrimitiveType(const Name& name, uint32_t size, uint32_t alignment, std::vector<FieldDecl*> fields)
{
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return nullptr;
    auto new_type = new TypeDecl(*this, name, size, alignment, fields, true);
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

FieldDecl* AST::DeclareField(const Name& name, const TypeDecl* type)
{
    auto decl = new FieldDecl(*this, name, type);
    _decls.emplace_back(decl);
    return decl;
}

FunctionDecl* AST::DeclareFunction(const Name& name, TypeDecl* const return_type, std::span<ParamVarDecl* const> params, CompoundStmt* body)
{
    auto decl = new FunctionDecl(*this, name, return_type, params, body);
    _decls.emplace_back(decl);
    _funcs.emplace_back(decl);
    return decl;
}

InitListExpr* AST::InitList(std::span<Expr*> exprs)
{
    auto expr = new InitListExpr(*this, exprs);
    _stmts.emplace_back(expr);
    return expr;
}

ParamVarDecl* AST::DeclareParam(const TypeDecl* type, const Name& name)
{
    auto decl = new ParamVarDecl(*this, type, name);
    _decls.emplace_back(decl);
    return decl;
}

DeclRefExpr* AST::Ref(const Decl* decl)
{
    auto expr = new DeclRefExpr(*this, *decl);
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

template <typename... Args>
std::vector<FieldDecl*> DeclareFields(AST* ast, const TypeDecl* type, Args&&... args)
{
    std::vector<FieldDecl*> fields;
    (fields.emplace_back(ast->DeclareField(std::forward<Args>(args), type)), ...);
    return fields;
}

#define USTR(x) L ## #x
#define INIT_BUILTIN_TYPE(symbol, type, name) symbol##Type(DeclarePrimitiveType(USTR(name), sizeof(type), alignof(type)))

#define INIT_VEC_TYPES(symbol, type, name) \
    symbol##2Type(DeclarePrimitiveType(USTR(name##2), sizeof(vec<type, 2>), alignof(vec<type, 2>), DeclareFields(this, symbol##Type, L"x"))), \
    symbol##3Type(DeclarePrimitiveType(USTR(name##3), sizeof(vec<type, 3>), alignof(vec<type, 3>), DeclareFields(this, symbol##Type, L"x", L"y"))), \
    symbol##4Type(DeclarePrimitiveType(USTR(name##4), sizeof(vec<type, 4>), alignof(vec<type, 4>), DeclareFields(this, symbol##Type, L"x", L"y", L"z")))

#define INIT_MATRIX_TYPE(symbol, type, name) \
    symbol##1x1Type(DeclarePrimitiveType(USTR(name##1x1), sizeof(matrix<type, 1, 1>), alignof(matrix<type, 1, 1>))), \
    symbol##1x2Type(DeclarePrimitiveType(USTR(name##1x2), sizeof(matrix<type, 1, 2>), alignof(matrix<type, 1, 2>))), \
    symbol##1x3Type(DeclarePrimitiveType(USTR(name##1x3), sizeof(matrix<type, 1, 3>), alignof(matrix<type, 1, 3>))), \
    symbol##1x4Type(DeclarePrimitiveType(USTR(name##1x4), sizeof(matrix<type, 1, 4>), alignof(matrix<type, 1, 4>))), \
    \
    symbol##2x1Type(DeclarePrimitiveType(USTR(name##2x1), sizeof(matrix<type, 2, 1>), alignof(matrix<type, 2, 1>))), \
    symbol##2x2Type(DeclarePrimitiveType(USTR(name##2x2), sizeof(matrix<type, 2, 2>), alignof(matrix<type, 2, 2>))), \
    symbol##2x3Type(DeclarePrimitiveType(USTR(name##2x3), sizeof(matrix<type, 2, 3>), alignof(matrix<type, 2, 3>))), \
    symbol##2x4Type(DeclarePrimitiveType(USTR(name##2x4), sizeof(matrix<type, 2, 4>), alignof(matrix<type, 2, 4>))), \
    \
    symbol##3x1Type(DeclarePrimitiveType(USTR(name##3x1), sizeof(matrix<type, 3, 1>), alignof(matrix<type, 3, 1>))), \
    symbol##3x2Type(DeclarePrimitiveType(USTR(name##3x2), sizeof(matrix<type, 3, 2>), alignof(matrix<type, 3, 2>))), \
    symbol##3x3Type(DeclarePrimitiveType(USTR(name##3x3), sizeof(matrix<type, 3, 3>), alignof(matrix<type, 3, 3>))), \
    symbol##3x4Type(DeclarePrimitiveType(USTR(name##3x4), sizeof(matrix<type, 3, 4>), alignof(matrix<type, 3, 4>))), \
    \
    symbol##4x1Type(DeclarePrimitiveType(USTR(name##4x1), sizeof(matrix<type, 4, 1>), alignof(matrix<type, 4, 1>))), \
    symbol##4x2Type(DeclarePrimitiveType(USTR(name##4x2), sizeof(matrix<type, 4, 2>), alignof(matrix<type, 4, 2>))), \
    symbol##4x3Type(DeclarePrimitiveType(USTR(name##4x3), sizeof(matrix<type, 4, 3>), alignof(matrix<type, 4, 3>))), \
    symbol##4x4Type(DeclarePrimitiveType(USTR(name##4x4), sizeof(matrix<type, 4, 4>), alignof(matrix<type, 4, 4>)))

AST::AST() : 
    VoidType(DeclarePrimitiveType(L"void", 0, 0)),
    
    INIT_BUILTIN_TYPE(Bool, GPUBool, bool),
    INIT_VEC_TYPES(Bool, GPUBool, bool),
    INIT_MATRIX_TYPE(Bool, GPUBool, bool),

    INIT_BUILTIN_TYPE(Float, float, float),
    INIT_VEC_TYPES(Float, float, float),
    INIT_MATRIX_TYPE(Float, float, float),

    INIT_BUILTIN_TYPE(Int, int32_t, int),
    INIT_VEC_TYPES(Int, int32_t, int),
    INIT_MATRIX_TYPE(Int, int32_t, int),

    INIT_BUILTIN_TYPE(UInt, uint32_t, uint),
    INIT_VEC_TYPES(UInt, uint32_t, uint),
    INIT_MATRIX_TYPE(UInt, uint32_t, uint),

    INIT_BUILTIN_TYPE(Double, double, double),
    INIT_BUILTIN_TYPE(I64, int64_t, int64),
    INIT_BUILTIN_TYPE(U64, uint64_t, uint64)
{
    
}

#undef INIT_MATRIX_TYPE
#undef INIT_VEC_TYPES
#undef INIT_BUILTIN_TYPE
#undef USTR

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