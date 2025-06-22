#include "SSL/AST.hpp"
#include <stdexcept>

namespace skr::SSL 
{

BinaryExpr* AST::Binary(BinaryOp op, Expr* left, Expr* right)
{
    auto expr = new BinaryExpr(*this, left, right, op);
    _stmts.emplace_back(expr);
    return expr;
}

BitwiseCastExpr* AST::BitwiseCast(const TypeDecl* type, Expr* expr)
{
    auto cast = new BitwiseCastExpr(*this, type, expr);
    _stmts.emplace_back(cast);
    return cast;
}

BreakStmt* AST::Break()
{
    auto stmt = new BreakStmt(*this);
    _stmts.emplace_back(stmt);
    return stmt;
}

CompoundStmt* AST::Block(const std::vector<Stmt*>& statements)
{
    auto exp = new CompoundStmt(*this, statements);
    _stmts.emplace_back(exp);
    return exp;
}

CallExpr* AST::CallFunction(DeclRefExpr* callee, std::span<Expr*> args)
{
    auto expr = new CallExpr(*this, callee, args);
    _stmts.emplace_back(expr);
    return expr;
}

CaseStmt* AST::Case(Expr* cond, CompoundStmt* body)
{
    auto stmt = new CaseStmt(*this, cond, body);
    _stmts.emplace_back(stmt);
    return stmt;
}

MethodCallExpr* AST::CallMethod(MemberExpr* callee, std::span<Expr*> args)
{
    auto expr = new MethodCallExpr(*this, callee, args);
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

ContinueStmt* AST::Continue()
{
    auto stmt = new ContinueStmt(*this);
    _stmts.emplace_back(stmt);
    return stmt;
}

DefaultStmt* AST::Default(CompoundStmt* body)
{
    auto stmt = new DefaultStmt(*this, body);
    _stmts.emplace_back(stmt);
    return stmt;
}

FieldExpr* AST::Field(Expr* base, const FieldDecl* field)
{
    auto expr = new FieldExpr(*this, base, field);
    _stmts.emplace_back(expr);
    return expr;
}

ForStmt* AST::For(Stmt* init, Expr* cond, Stmt* inc, CompoundStmt* body)
{
    auto stmt = new ForStmt(*this, init, cond, inc, body);
    _stmts.emplace_back(stmt);
    return stmt;
}

IfStmt* AST::If(Expr* cond, CompoundStmt* then_body, CompoundStmt* else_body)
{
    auto stmt = new IfStmt(*this, cond, then_body, else_body);
    _stmts.emplace_back(stmt);
    return stmt;
}

InitListExpr* AST::InitList(std::span<Expr*> exprs)
{
    auto expr = new InitListExpr(*this, exprs);
    _stmts.emplace_back(expr);
    return expr;
}

ImplicitCastExpr* AST::ImplicitCast(const TypeDecl* type, Expr* expr)
{
    auto cast = new ImplicitCastExpr(*this, type, expr);
    _stmts.emplace_back(cast);
    return cast;
}

MethodExpr* AST::Method(DeclRefExpr* base, const MethodDecl* method)
{
    auto expr = new MethodExpr(*this, base, method);
    _stmts.emplace_back(expr);
    return expr;
}

DeclRefExpr* AST::Ref(const Decl* decl)
{
    assert(decl && "DeclRefExpr cannot be created with a null decl");
    auto expr = new DeclRefExpr(*this, *decl);
    _stmts.emplace_back(expr);
    return expr;
}

ReturnStmt* AST::Return(Expr* expr)
{
    auto stmt = new ReturnStmt(*this, expr);
    _stmts.emplace_back(stmt);
    return stmt;
}

StaticCastExpr* AST::StaticCast(const TypeDecl* type, Expr* expr)
{
    auto cast = new StaticCastExpr(*this, type, expr);
    _stmts.emplace_back(cast);
    return cast;
}

SwizzleExpr* AST::Swizzle(Expr* expr, uint64_t comps, const uint64_t* seq)
{
    auto swizzle = new SwizzleExpr(*this, expr, comps, seq);
    _stmts.emplace_back(swizzle);
    return swizzle;
}

SwitchStmt* AST::Switch(Expr* cond, std::span<CaseStmt*> cases)
{
    auto stmt = new SwitchStmt(*this, cond, cases);
    _stmts.emplace_back(stmt);
    return stmt;
}

ThisExpr* AST::This()
{
    auto expr = new ThisExpr(*this);
    _stmts.emplace_back(expr);
    return expr;
}

UnaryExpr* AST::Unary(UnaryOp op, Expr* expr)
{
    auto unary = new UnaryExpr(*this, op, expr);
    _stmts.emplace_back(unary);
    return unary;
}

DeclStmt* AST::Variable(EVariableQualifier qualifier, const TypeDecl* type, Expr* initializer) 
{  
    auto decl_name = L"decl" + std::to_wstring(_decls.size());
    return Variable(qualifier, type, decl_name, initializer); 
}

DeclStmt* AST::Variable(EVariableQualifier qualifier, const TypeDecl* type, const Name& name, Expr* initializer) 
{  
    assert(qualifier != EVariableQualifier::Inout && "Inout qualifier is not allowed for variable declarations");
    
    auto decl = new VarDecl(*this, qualifier, type, name, initializer);
    _decls.emplace_back(decl);

    auto stmt = new DeclStmt(*this, decl);
    _stmts.emplace_back(stmt);

    return stmt;
}

WhileStmt* AST::While(Expr* cond, CompoundStmt* body)
{
    auto stmt = new WhileStmt(*this, cond, body);
    _stmts.emplace_back(stmt);
    return stmt;
}

TypeDecl* AST::DeclareType(const Name& name, std::span<FieldDecl*> fields)
{
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return nullptr;
    auto new_type = new TypeDecl(*this, name, fields, false);
    _types.emplace_back(new_type);
    return new_type;
}

const TypeDecl* AST::DeclarePrimitiveType(const Name& name, uint32_t size, uint32_t alignment, std::vector<FieldDecl*> fields)
{
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return nullptr;
    auto new_type = new TypeDecl(*this, name, size, alignment, fields, true);
    _types.emplace_back(new_type);
    return new_type;
}

const ArrayTypeDecl* AST::DeclareArrayType(const TypeDecl* element, uint32_t count)
{
    auto found = _arrs.find({element, count});
    if (found != _arrs.end())
        return found->second;
    auto new_type = new ArrayTypeDecl(*this, element, count);
    _types.emplace_back(new_type);
    _arrs.insert({{element, count}, new_type});
    return new_type;
}

GlobalVarDecl* AST::DeclareGlobalConstant(const TypeDecl* type, const Name& name, ConstantExpr* initializer)
{
    // TODO: CHECK THIS IS NOT RESOURCE TYPE
    auto decl = new GlobalVarDecl(*this, EVariableQualifier::Const, type, name, initializer);
    _decls.emplace_back(decl);
    _globals.emplace_back(decl);
    return decl;
}

GlobalVarDecl* AST::DeclareGlobalResource(const TypeDecl* type, const Name& name)
{
    // TODO: CHECK THIS IS RESOURCE TYPE
    auto decl = new GlobalVarDecl(*this, EVariableQualifier::None, type, name, nullptr);
    _decls.emplace_back(decl);
    _globals.emplace_back(decl);
    return decl;
}

FieldDecl* AST::DeclareField(const Name& name, const TypeDecl* type)
{
    auto decl = new FieldDecl(*this, name, type);
    _decls.emplace_back(decl);
    return decl;
}

MethodDecl* AST::DeclareMethod(TypeDecl* owner, const Name& name, const TypeDecl* return_type, std::span<const ParamVarDecl* const> params, CompoundStmt* body)
{
    auto decl = new MethodDecl(*this, owner, name, return_type, params, body);
    _decls.emplace_back(decl);
    _methods.emplace_back(decl);
    return decl;
}

ConstructorDecl* AST::DeclareConstructor(TypeDecl* owner, const Name& name, std::span<const ParamVarDecl* const> params, CompoundStmt* body)
{
    auto decl = new ConstructorDecl(*this, owner, name, params, body);
    _decls.emplace_back(decl);
    _ctors.emplace_back(decl);
    return decl;
}

FunctionDecl* AST::DeclareFunction(const Name& name, const TypeDecl* return_type, std::span<const ParamVarDecl* const> params, CompoundStmt* body)
{
    auto decl = new FunctionDecl(*this, name, return_type, params, body);
    _decls.emplace_back(decl);
    _funcs.emplace_back(decl);
    return decl;
}

ParamVarDecl* AST::DeclareParam(EVariableQualifier qualifier, const TypeDecl* type, const Name& name)
{
    auto decl = new ParamVarDecl(*this, qualifier, type, name);
    _decls.emplace_back(decl);
    return decl;
}

ByteBufferTypeDecl* AST::ByteBuffer(BufferFlags flags)
{
    auto&& iter = _buffers.find(nullptr);
    if (iter != _buffers.end())
        return dynamic_cast<ByteBufferTypeDecl*>(iter->second);

    auto new_type = new ByteBufferTypeDecl(*this, flags);
    _types.emplace_back(new_type);
    _buffers[nullptr] = new_type;
    return new_type;
}

StructuredBufferTypeDecl* AST::StructuredBuffer(const TypeDecl* element, BufferFlags flags)
{
    auto&& iter = _buffers.find(element);
    if (iter != _buffers.end())
        return dynamic_cast<StructuredBufferTypeDecl*>(iter->second);

    auto new_type = new StructuredBufferTypeDecl(*this, element, flags);
    _types.emplace_back(new_type);
    _buffers[element] = new_type;
    return new_type;
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
    symbol##2Type(DeclarePrimitiveType(USTR(name##2), sizeof(vec<type, 2>), alignof(vec<type, 2>), DeclareFields(this, symbol##Type, L"x", L"y"))), \
    symbol##3Type(DeclarePrimitiveType(USTR(name##3), sizeof(vec<type, 3>), alignof(vec<type, 3>), DeclareFields(this, symbol##Type, L"x", L"y", L"z"))), \
    symbol##4Type(DeclarePrimitiveType(USTR(name##4), sizeof(vec<type, 4>), alignof(vec<type, 4>), DeclareFields(this, symbol##Type, L"x", L"y", L"z", L"w")))

#define INIT_MATRIX_TYPE(symbol, type, name) \
    symbol##2x2Type(DeclarePrimitiveType(USTR(name##2x2), sizeof(matrix<type, 2, 2>), alignof(matrix<type, 2, 2>))), \
    symbol##3x3Type(DeclarePrimitiveType(USTR(name##3x3), sizeof(matrix<type, 3, 3>), alignof(matrix<type, 3, 3>))), \
    symbol##4x4Type(DeclarePrimitiveType(USTR(name##4x4), sizeof(matrix<type, 4, 4>), alignof(matrix<type, 4, 4>)))

AST::AST() : 
    VoidType(DeclarePrimitiveType(L"void", 0, 0)),
    
    INIT_BUILTIN_TYPE(Bool, GPUBool, bool),
    INIT_VEC_TYPES(Bool, GPUBool, bool),
    // INIT_MATRIX_TYPE(Bool, GPUBool, bool),

    INIT_BUILTIN_TYPE(Half, float, half),

    INIT_BUILTIN_TYPE(Float, float, float),
    INIT_VEC_TYPES(Float, float, float),
    INIT_MATRIX_TYPE(Float, float, float),

    INIT_BUILTIN_TYPE(Int, int32_t, int),
    INIT_VEC_TYPES(Int, int32_t, int),
    // INIT_MATRIX_TYPE(Int, int32_t, int),

    INIT_BUILTIN_TYPE(UInt, uint32_t, uint),
    INIT_VEC_TYPES(UInt, uint32_t, uint),
    // INIT_MATRIX_TYPE(UInt, uint32_t, uint),

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

    for (auto attr : _attrs)
    {
        delete attr;
    }
    _attrs.clear();
}

} // namespace skr::SSL