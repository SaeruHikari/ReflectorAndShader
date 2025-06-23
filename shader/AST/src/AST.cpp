#include "SSL/AST.hpp"
#include <array>
#include <unordered_set>

namespace skr::SSL 
{

struct VarConcept : public VarConceptDecl
{
    VarConcept(AST& ast, const Name& name, std::function<bool(EVariableQualifier, const TypeDecl*)> validator)
        : VarConceptDecl(ast, name), validator(std::move(validator))
    {

    }
    bool validate(EVariableQualifier qualifier, const TypeDecl* type) const override
    {
        return validator(qualifier, type);
    }
    std::function<bool(EVariableQualifier, const TypeDecl*)> validator;
};

struct TemplateCallable : public TemplateCallableDecl
{
    TemplateCallable(AST& ast, const Name& name, ReturnTypeSpecializer ret_spec, std::span<const VarConceptDecl* const> param_concepts)
        : TemplateCallableDecl(ast, name, param_concepts), ret_spec(ret_spec)
    {

    }
    TemplateCallable(AST& ast, TypeDecl* owner, const Name& name, ReturnTypeSpecializer ret_spec, std::span<const VarConceptDecl* const> param_concepts)
        : TemplateCallableDecl(ast, owner, name, param_concepts), ret_spec(ret_spec)
    {

    }
    const TypeDecl* get_return_type_for(std::span<const TypeDecl* const> arg_types) const
    {
        return ret_spec(arg_types);
    }
    ReturnTypeSpecializer ret_spec;
};

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

SwizzleExpr* AST::Swizzle(Expr* expr, const TypeDecl* type, uint64_t comps, const uint64_t* seq)
{
    auto swizzle = new SwizzleExpr(*this, expr, type, comps, seq);
    _stmts.emplace_back(swizzle);
    return swizzle;
}

SwitchStmt* AST::Switch(Expr* cond, std::span<CaseStmt*> cases)
{
    auto stmt = new SwitchStmt(*this, cond, cases);
    _stmts.emplace_back(stmt);
    return stmt;
}

ThisExpr* AST::This(const TypeDecl* type)
{
    auto expr = new ThisExpr(*this, type);
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
    ReservedWordsCheck(name);
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
    ReservedWordsCheck(name);
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
    ReservedWordsCheck(name);
    auto decl = new GlobalVarDecl(*this, EVariableQualifier::Const, type, name, initializer);
    _decls.emplace_back(decl);
    _globals.emplace_back(decl);
    return decl;
}

GlobalVarDecl* AST::DeclareGlobalResource(const TypeDecl* type, const Name& name)
{
    // TODO: CHECK THIS IS RESOURCE TYPE
    ReservedWordsCheck(name);
    auto decl = new GlobalVarDecl(*this, EVariableQualifier::None, type, name, nullptr);
    _decls.emplace_back(decl);
    _globals.emplace_back(decl);
    return decl;
}

FieldDecl* AST::DeclareField(const Name& name, const TypeDecl* type)
{
    ReservedWordsCheck(name);
    auto decl = new FieldDecl(*this, name, type);
    _decls.emplace_back(decl);
    return decl;
}

MethodDecl* AST::DeclareMethod(TypeDecl* owner, const Name& name, const TypeDecl* return_type, std::span<const ParamVarDecl* const> params, CompoundStmt* body)
{
    ReservedWordsCheck(name);
    auto decl = new MethodDecl(*this, owner, name, return_type, params, body);
    _decls.emplace_back(decl);
    _methods.emplace_back(decl);
    return decl;
}

ConstructorDecl* AST::DeclareConstructor(TypeDecl* owner, const Name& name, std::span<const ParamVarDecl* const> params, CompoundStmt* body)
{
    ReservedWordsCheck(name);
    auto decl = new ConstructorDecl(*this, owner, name, params, body);
    _decls.emplace_back(decl);
    _ctors.emplace_back(decl);
    return decl;
}

FunctionDecl* AST::DeclareFunction(const Name& name, const TypeDecl* return_type, std::span<const ParamVarDecl* const> params, CompoundStmt* body)
{
    ReservedWordsCheck(name);
    auto decl = new FunctionDecl(*this, name, return_type, params, body);
    _decls.emplace_back(decl);
    _funcs.emplace_back(decl);
    return decl;
}

ParamVarDecl* AST::DeclareParam(EVariableQualifier qualifier, const TypeDecl* type, const Name& name)
{
    ReservedWordsCheck(name);
    auto decl = new ParamVarDecl(*this, qualifier, type, name);
    _decls.emplace_back(decl);
    return decl;
}

VarConceptDecl* AST::DeclareVarConcept(const Name& name, std::function<bool(EVariableQualifier, const TypeDecl*)> validator)
{
    auto decl = new VarConcept(*this, name, std::move(validator));
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

Texture2DTypeDecl* AST::Texture2D(const TypeDecl* element, TextureFlags flags)
{
    auto&& iter = _texture2ds.find(element);
    if (iter != _texture2ds.end())
        return dynamic_cast<Texture2DTypeDecl*>(iter->second);

    auto new_type = new Texture2DTypeDecl(*this, element, flags);
    _types.emplace_back(new_type);
    _texture2ds[element] = new_type;
    return new_type;
}

Texture3DTypeDecl* AST::Texture3D(const TypeDecl* element, TextureFlags flags)
{
    auto&& iter = _texture3ds.find(element);
    if (iter != _texture3ds.end())
        return dynamic_cast<Texture3DTypeDecl*>(iter->second);

    auto new_type = new Texture3DTypeDecl(*this, element, flags);
    _types.emplace_back(new_type);
    _texture3ds[element] = new_type;
    return new_type;
}

const TypeDecl* AST::GetType(const Name& name) const
{
    ReservedWordsCheck(name);
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

// Template function/method declarations
TemplateCallableDecl* AST::DeclareTemplateFunction(const Name& name, const TypeDecl* return_type, std::span<const VarConceptDecl* const> param_concepts)
{
    auto decl = new TemplateCallable(*this, name, [=](auto params){ return return_type; }, param_concepts);
    _decls.emplace_back(decl);
    return decl;
}

TemplateCallableDecl* AST::DeclareTemplateFunction(const Name& name, TemplateCallableDecl::ReturnTypeSpecializer ret_spec, std::span<const VarConceptDecl* const> param_concepts)
{
    auto decl = new TemplateCallable(*this, name, ret_spec, param_concepts);
    _decls.emplace_back(decl);
    return decl;
}

TemplateCallableDecl* AST::DeclareTemplateMethod(TypeDecl* owner, const Name& name, const TypeDecl* return_type, std::span<const VarConceptDecl* const> param_concepts)
{
    auto decl = new TemplateCallable(*this, owner, name, [=](auto params){ return return_type; }, param_concepts);
    _decls.emplace_back(decl);
    return decl;
}

TemplateCallableDecl* AST::DeclareTemplateMethod(TypeDecl* owner, const Name& name, TemplateCallableDecl::ReturnTypeSpecializer ret_spec, std::span<const VarConceptDecl* const> param_concepts)
{
    auto decl = new TemplateCallable(*this, owner, name, ret_spec, param_concepts);
    _decls.emplace_back(decl);
    return decl;
}

SpecializedFunctionDecl* AST::SpecializeTemplateFunction(const TemplateCallableDecl* template_decl, std::span<const TypeDecl* const> arg_types, std::span<const EVariableQualifier> arg_qualifiers)
{
    // Create new specialization
    auto specialized = (SpecializedFunctionDecl*)template_decl->specialize_for(arg_types, arg_qualifiers);
    _decls.emplace_back(specialized);
    return specialized;
}

const TemplateCallableDecl* AST::FindIntrinsic(const char* name) const
{
    auto it = _template_intrinstics.find(name);
    if (it != _template_intrinstics.end())
        return it->second;
    return nullptr;
}

SpecializedMethodDecl* AST::SpecializeTemplateMethod(const TemplateCallableDecl* template_decl, std::span<const TypeDecl* const> arg_types, std::span<const EVariableQualifier> arg_qualifiers)
{
    auto specialized = (SpecializedMethodDecl*)template_decl->specialize_for(arg_types, arg_qualifiers);
    _decls.emplace_back(specialized);
    return specialized;
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
    // INIT_MATRIX_TYPE(UInt, uint32_t, uint),    INIT_BUILTIN_TYPE(Double, double, double),
    INIT_BUILTIN_TYPE(I64, int64_t, int64),
    INIT_BUILTIN_TYPE(U64, uint64_t, uint64)
{
    DoubleType = FloatType; // Shaders normally does not support double, so we use FloatType for DoubleType

    auto Vector2D = DeclareVarConcept(L"Vector2D", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float2Type || type == Int2Type || type == UInt2Type || type == Bool2Type;
        });
    auto Vector3D = DeclareVarConcept(L"Vector3D",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float3Type || type == Int3Type || type == UInt3Type || type == Bool3Type;
        });
    auto Vector4D = DeclareVarConcept(L"Vector4D",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float4Type || type == Int4Type || type == UInt4Type || type == Bool4Type;
        });

    auto IntScalar = DeclareVarConcept(L"IntScalar", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == IntType || type == UIntType || type == I64Type || type == U64Type;
        });
    auto IntVector = DeclareVarConcept(L"IntVector", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Int2Type || type == Int3Type || type == Int4Type ||
                   type == UInt2Type || type == UInt3Type || type == UInt4Type;
        });
        
    auto FloatScalar = DeclareVarConcept(L"FloatScalar",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == FloatType || type == HalfType;
        });
    auto FloatVector = DeclareVarConcept(L"FloatVector",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float2Type || type == Float3Type || type == Float4Type;
        });
    auto FloatVector2D = DeclareVarConcept(L"FloatVector2D",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float2Type;
        });
    auto FloatVector3D = DeclareVarConcept(L"FloatVector3D",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float3Type;
        });
    auto FloatVector4D = DeclareVarConcept(L"FloatVector4D",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float4Type;
        });

    auto ResourceFamily = DeclareVarConcept(L"ResourceFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return dynamic_cast<const skr::SSL::BufferTypeDecl*>(type) != nullptr;
        });
    auto ValueFamily = DeclareVarConcept(L"ValueFamily", 
        [this, ResourceFamily](EVariableQualifier qualifier, const TypeDecl* type) {
            return !ResourceFamily->validate(qualifier, type);
        });

    auto IntFamily = DeclareVarConcept(L"IntFamily", 
        [this, IntScalar, IntVector](EVariableQualifier qualifier, const TypeDecl* type) {
            return IntScalar->validate(qualifier, type) || IntVector->validate(qualifier, type);
        });
    auto FloatFamily = DeclareVarConcept(L"FloatFamily", 
        [this, FloatScalar, FloatVector](EVariableQualifier qualifier, const TypeDecl* type) {
            return FloatScalar->validate(qualifier, type) || FloatVector->validate(qualifier, type);
        });
    auto BoolFamily = DeclareVarConcept(L"BoolFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == BoolType || type == Bool2Type || type == Bool3Type || type == Bool4Type;
        });
    auto ArthmeticFamily = DeclareVarConcept(L"ArithmeticFamily", 
        [this, IntFamily, FloatFamily](EVariableQualifier qualifier, const TypeDecl* type) {
            return IntFamily->validate(qualifier, type) || FloatFamily->validate(qualifier, type);
        });

    auto MatrixFamily = DeclareVarConcept(L"MatrixFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float2x2Type || type == Float3x3Type || type == Float4x4Type;
        });

    auto BufferFamily = DeclareVarConcept(L"BufferFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type)  {
            return (dynamic_cast<const skr::SSL::BufferTypeDecl*>(type) != nullptr);
        });
    auto StructuredBufferFamily = DeclareVarConcept(L"StructuredBufferFamily",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return (dynamic_cast<const skr::SSL::StructuredBufferTypeDecl*>(type) != nullptr);
        });

    auto TextureFamily = DeclareVarConcept(L"TextureFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return (dynamic_cast<const skr::SSL::TextureTypeDecl*>(type) != nullptr);
        });
    auto FloatTexture2DFamily = DeclareVarConcept(L"FloatTextureFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            auto t = dynamic_cast<const skr::SSL::Texture2DTypeDecl*>(type);
            return (t != nullptr) && (&t->element() == FloatType);
        });
    auto FloatTexture3DFamily = DeclareVarConcept(L"FloatTexture3DFamily",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            auto t = dynamic_cast<const skr::SSL::Texture3DTypeDecl*>(type);
            return (t != nullptr) && (&t->element() == FloatType);
        });

    auto ReturnSpec_PickFirst = [=](auto pts){ return pts[0]; };

    std::array<VarConceptDecl*, 1> OneArithmetic = { ArthmeticFamily };
    std::array<VarConceptDecl*, 2> TwoArithmetic = { ArthmeticFamily, ArthmeticFamily };
    std::array<VarConceptDecl*, 3> ThreeArithmetic = { ArthmeticFamily, ArthmeticFamily, ArthmeticFamily };
    _template_intrinstics["ABS"] = DeclareTemplateFunction(L"abs", ReturnSpec_PickFirst, OneArithmetic);
    _template_intrinstics["CLAMP"] = DeclareTemplateFunction(L"clamp", ReturnSpec_PickFirst, ThreeArithmetic);
    _template_intrinstics["LERP"] = DeclareTemplateFunction(L"lerp", ReturnSpec_PickFirst, ThreeArithmetic);

    std::array<VarConceptDecl*, 1> OneFloatFamily = { FloatFamily };
    _template_intrinstics["SIN"] = DeclareTemplateFunction(L"sin", ReturnSpec_PickFirst, OneFloatFamily);
    _template_intrinstics["SINH"] = DeclareTemplateFunction(L"sinh", ReturnSpec_PickFirst, OneFloatFamily);
    _template_intrinstics["COS"] = DeclareTemplateFunction(L"cos", ReturnSpec_PickFirst, OneFloatFamily);
    _template_intrinstics["COSH"] = DeclareTemplateFunction(L"cosh", ReturnSpec_PickFirst, OneFloatFamily);
    _template_intrinstics["ATAN"] = DeclareTemplateFunction(L"atan", ReturnSpec_PickFirst, OneFloatFamily);
    _template_intrinstics["ATANH"] = DeclareTemplateFunction(L"atanh", ReturnSpec_PickFirst, OneFloatFamily);
    _template_intrinstics["TAN"] = DeclareTemplateFunction(L"tan", ReturnSpec_PickFirst, OneFloatFamily);
    _template_intrinstics["TANH"] = DeclareTemplateFunction(L"tanh", ReturnSpec_PickFirst, OneFloatFamily);
    _template_intrinstics["LENGTH"] = DeclareTemplateFunction(L"length", FloatType, OneFloatFamily);
    _template_intrinstics["LOG10"] = DeclareTemplateFunction(L"log10", ReturnSpec_PickFirst, OneFloatFamily);
    _template_intrinstics["SATURATE"] = DeclareTemplateFunction(L"saturate", ReturnSpec_PickFirst, OneFloatFamily);

    std::array<VarConceptDecl*, 2> TwoFloatFamily = { FloatFamily, FloatFamily };
    _template_intrinstics["POW"] = DeclareTemplateFunction(L"pow", ReturnSpec_PickFirst, TwoFloatFamily);

    std::array<VarConceptDecl*, 2> TwoFloatVec = { FloatVector, FloatVector };
    _template_intrinstics["DOT"] = DeclareTemplateFunction(L"dot", FloatType, TwoFloatVec);

    std::array<VarConceptDecl*, 1> OneMatrix = { MatrixFamily };
    _template_intrinstics["TRANSPOSE"] = DeclareTemplateFunction(L"transpose", ReturnSpec_PickFirst, OneMatrix);

    std::array<VarConceptDecl*, 2> BufferReadParams = { BufferFamily, IntScalar };
    _template_intrinstics["BUFFER_READ"] = DeclareTemplateFunction(L"buffer_read", 
        [=](auto pts) { 
            return &dynamic_cast<const StructuredBufferTypeDecl*>(pts[0])->element(); 
        }, BufferReadParams);

    std::array<VarConceptDecl*, 3> BufferWriteParams = { BufferFamily, IntScalar, ValueFamily };
    _template_intrinstics["BUFFER_WRITE"] = DeclareTemplateFunction(L"buffer_write", VoidType, BufferWriteParams);

    std::array<VarConceptDecl*, 1> TextureSizeParams = { TextureFamily };
    _template_intrinstics["TEXTURE_SIZE"] = DeclareTemplateFunction(L"texture_size", UInt3Type, TextureSizeParams);

    std::array<VarConceptDecl*, 2> TextureReadParams = { TextureFamily, IntScalar };
    _template_intrinstics["TEXTURE_READ"] = DeclareTemplateFunction(L"texture_read", 
        [=](auto pts) { 
            return &dynamic_cast<const TextureTypeDecl*>(pts[0])->element(); 
        }, TextureReadParams);
    
    std::array<VarConceptDecl*, 3> TextureWriteParams = { TextureFamily, IntVector, Vector4D };
    _template_intrinstics["TEXTURE_WRITE"] = DeclareTemplateFunction(L"texture_write", VoidType, TextureWriteParams);

    std::array<VarConceptDecl*, 4> Texture2DSampleParams = { FloatTexture2DFamily, FloatVector2D/*uv*/, IntScalar/*filter*/, IntScalar/*address*/ };
    _template_intrinstics["TEXTURE2D_SAMPLE"] = DeclareTemplateFunction(L"texture2d_sample", Float4Type, Texture2DSampleParams);

    std::array<VarConceptDecl*, 4> Texture3DSampleParams = { FloatTexture3DFamily, FloatVector3D/*uv*/, IntScalar/*filter*/, IntScalar/*address*/ };
    _template_intrinstics["TEXTURE3D_SAMPLE"] = DeclareTemplateFunction(L"texture3d_sample", Float4Type, Texture3DSampleParams);

}

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

template <typename... Args>
[[noreturn]] void AST::ReportFatalError(std::wformat_string<Args...> fmt, Args&&... args) const
{
    ReportFatalError(std::format(fmt, std::forward<Args>(args)...));
}

[[noreturn]] void AST::ReportFatalError(const String& message) const
{
    std::wcerr << dump() << std::endl;
    std::wcerr << message << std::endl;
    abort();
}

void AST::ReservedWordsCheck(const Name& name) const
{
    static const std::unordered_set<Name> reserved_names = {
        L"float", L"int", L"uint", L"bool", L"void",
        L"half", L"double", L"int64_t", L"uint64_t"
    };
    if (reserved_names.contains(name))
    {
        ReportFatalError(L"{} is a reserved word, which should not be used!", name);
    }
}

} // namespace skr::SSL