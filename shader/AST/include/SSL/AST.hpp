#pragma once
#include <map>
#include <unordered_map>
#include "Attr.hpp"
#include "Expr.hpp"
#include "Decl.hpp"
#include "Constant.hpp"

namespace skr::SSL {

#define VEC_TYPES(N) TypeDecl* const N##2Type = nullptr; TypeDecl* const N##3Type = nullptr; TypeDecl* const N##4Type = nullptr;
#define MATRIX_TYPES(N) \
    TypeDecl* const N##1x1Type = nullptr; TypeDecl* const N##1x2Type = nullptr; TypeDecl* const N##1x3Type = nullptr; TypeDecl* const N##1x4Type = nullptr;\
    TypeDecl* const N##2x1Type = nullptr; TypeDecl* const N##2x2Type = nullptr; TypeDecl* const N##2x3Type = nullptr; TypeDecl* const N##2x4Type = nullptr;\
    TypeDecl* const N##3x1Type = nullptr; TypeDecl* const N##3x2Type = nullptr; TypeDecl* const N##3x3Type = nullptr; TypeDecl* const N##3x4Type = nullptr;\
    TypeDecl* const N##4x1Type = nullptr; TypeDecl* const N##4x2Type = nullptr; TypeDecl* const N##4x3Type = nullptr; TypeDecl* const N##4x4Type = nullptr;

struct AST
{
public:
    AST();
    ~AST();

    BinaryExpr* Binary(BinaryOp op, Expr* left, Expr* right);
    BitwiseCastExpr* BitwiseCast(const TypeDecl* type, Expr* expr);
    BreakStmt* Break();
    CompoundStmt* Block(const std::vector<Stmt*>& statements);
    CallExpr* CallFunction(DeclRefExpr* callee, std::span<Expr*> args);
    CaseStmt* Case(Expr* cond, CompoundStmt* body);
    MethodCallExpr* CallMethod(MemberExpr* callee, std::span<Expr*> args);
    ConstantExpr* Constant(const IntValue& v);
    ConstantExpr* Constant(const FloatValue& v);
    ConstructExpr* Construct(const TypeDecl* type, std::span<Expr*> args);
    ContinueStmt* Continue();
    DefaultStmt* Default(CompoundStmt* body);
    FieldExpr* Field(Expr* base, const FieldDecl* field);
    ForStmt* For(Stmt* init, Expr* cond, Stmt* inc, CompoundStmt* body);
    IfStmt* If(Expr* cond, CompoundStmt* then_body, CompoundStmt* else_body = nullptr);
    InitListExpr* InitList(std::span<Expr*> exprs);
    ImplicitCastExpr* ImplicitCast(const TypeDecl* type, Expr* expr);
    MethodExpr* Method(DeclRefExpr* base, const MethodDecl* method);
    DeclRefExpr* Ref(const Decl* decl);
    ReturnStmt* Return(Expr* expr);
    StaticCastExpr* StaticCast(const TypeDecl* type, Expr* expr);
    SwizzleExpr* Swizzle(Expr* expr, uint64_t comps, const uint64_t* seq);
    SwitchStmt* Switch(Expr* cond, std::span<CaseStmt*> cases);
    ThisExpr* This();
    UnaryExpr* Unary(UnaryOp op, Expr* expr);
    DeclStmt* Variable(EVariableQualifier qualifier, const TypeDecl* type, Expr* initializer = nullptr);
    DeclStmt* Variable(EVariableQualifier qualifier, const TypeDecl* type, const Name& name, Expr* initializer = nullptr);
    WhileStmt* While(Expr* cond, CompoundStmt* body);

    TypeDecl* const DeclareType(const Name& name, std::span<FieldDecl*> members);
    TypeDecl* const DeclarePrimitiveType(const Name& name, uint32_t size, uint32_t alignment = 4, std::vector<FieldDecl*> fields = {});
    ArrayTypeDecl* const DeclareArrayType(TypeDecl* const element, uint32_t count);
    GlobalVarDecl* const DeclareGlobalConstant(const TypeDecl* type, const Name& name, ConstantExpr* initializer = nullptr);
    GlobalVarDecl* const DeclareGlobalResource(const TypeDecl* type, const Name& name);
    FieldDecl* DeclareField(const Name& name, const TypeDecl* type);
    MethodDecl* DeclareMethod(TypeDecl* owner, const Name& name, TypeDecl* const return_type, std::span<ParamVarDecl* const> params, CompoundStmt* body);
    ConstructorDecl* DeclareConstructor(TypeDecl* owner, const Name& name, std::span<ParamVarDecl* const> params, CompoundStmt* body);
    FunctionDecl* DeclareFunction(const Name& name, TypeDecl* const return_type, std::span<ParamVarDecl* const> params, CompoundStmt* body);
    ParamVarDecl* DeclareParam(EVariableQualifier qualifier, const TypeDecl* type, const Name& name);

    ByteBufferTypeDecl* const ByteBuffer(BufferFlags flags);
    // TODO: for scalar, float2/4 types
    // Buffer* const Buffer(TypeDecl* const element, BufferFlags flags);
    StructuredBufferTypeDecl* const StructuredBuffer(TypeDecl* const element, BufferFlags flags);
    // TypeDecl* const TextureType(TypeDecl* const element);

    template <typename ATTR, typename... Args>
    inline ATTR* DeclareAttr(Args&&... args) {
        auto attr = new ATTR(std::forward<Args>(args)...);
        _attrs.emplace_back(attr);
        return attr;
    }

    inline BinaryExpr* Add(Expr* left, Expr* right) { return Binary(BinaryOp::ADD, left, right); }
    inline BinaryExpr* Sub(Expr* left, Expr* right) { return Binary(BinaryOp::SUB, left, right); }
    inline BinaryExpr* Mul(Expr* left, Expr* right) { return Binary(BinaryOp::MUL, left, right); }
    inline BinaryExpr* Div(Expr* left, Expr* right) { return Binary(BinaryOp::DIV, left, right); }
    inline BinaryExpr* Mod(Expr* left, Expr* right) { return Binary(BinaryOp::MOD, left, right); }
    inline BinaryExpr* BitAnd(Expr* left, Expr* right) { return Binary(BinaryOp::BIT_AND, left, right); }
    inline BinaryExpr* BitOr(Expr* left, Expr* right) { return Binary(BinaryOp::BIT_OR, left, right); }
    inline BinaryExpr* BitXor(Expr* left, Expr* right) { return Binary(BinaryOp::BIT_XOR, left, right); }
    inline BinaryExpr* Shl(Expr* left, Expr* right) { return Binary(BinaryOp::SHL, left, right); }
    inline BinaryExpr* Shr(Expr* left, Expr* right) { return Binary(BinaryOp::SHR, left, right); }
    inline BinaryExpr* And(Expr* left, Expr* right) { return Binary(BinaryOp::AND, left, right); }
    inline BinaryExpr* Or(Expr* left, Expr* right) { return Binary(BinaryOp::OR, left, right); }
    inline BinaryExpr* Less(Expr* left, Expr* right) { return Binary(BinaryOp::LESS, left, right); }
    inline BinaryExpr* Greater(Expr* left, Expr* right) { return Binary(BinaryOp::GREATER, left, right); }
    inline BinaryExpr* LessEqual(Expr* left, Expr* right) { return Binary(BinaryOp::LESS_EQUAL, left, right); }
    inline BinaryExpr* GreaterEqual(Expr* left, Expr* right) { return Binary(BinaryOp::GREATER_EQUAL, left, right); }
    inline BinaryExpr* Equal(Expr* left, Expr* right) { return Binary(BinaryOp::EQUAL, left, right); }
    inline BinaryExpr* NotEqual(Expr* left, Expr* right) { return Binary(BinaryOp::NOT_EQUAL, left, right); }
    inline BinaryExpr* Assign(Expr* left, Expr* right) { return Binary(BinaryOp::ASSIGN, left, right); }
    inline BinaryExpr* AddAssign(Expr* left, Expr* right) { return Binary(BinaryOp::ADD_ASSIGN, left, right); }
    inline BinaryExpr* SubAssign(Expr* left, Expr* right) { return Binary(BinaryOp::SUB_ASSIGN, left, right); }
    inline BinaryExpr* MulAssign(Expr* left, Expr* right) { return Binary(BinaryOp::MUL_ASSIGN, left, right); }
    inline BinaryExpr* DivAssign(Expr* left, Expr* right) { return Binary(BinaryOp::DIV_ASSIGN, left, right); }
    inline BinaryExpr* ModAssign(Expr* left, Expr* right) { return Binary(BinaryOp::MOD_ASSIGN, left, right); }

    const TypeDecl* GetType(const Name& name) const;

    std::span<Decl* const> decls() const { return _decls; }
    std::span<Stmt* const> stmts() const { return _stmts; }
    std::span<TypeDecl* const> types() const { return _types; }
    std::span<GlobalVarDecl* const> global_vars() const { return _globals; }
    std::span<FunctionDecl* const> funcs() const { return _funcs; }

    String dump() const;

private:
    std::vector<Decl*> _decls;
    std::vector<Stmt*> _stmts;
    std::unordered_map<TypeDecl*, BufferTypeDecl*> _buffers;
    std::vector<TypeDecl*> _types;
    std::vector<GlobalVarDecl*> _globals;
    std::vector<FunctionDecl*> _funcs;
    std::vector<MethodDecl*> _methods;
    std::vector<ConstructorDecl*> _ctors;
    std::vector<Attr*> _attrs;
    std::map<std::pair<TypeDecl*, uint32_t>, ArrayTypeDecl*> _arrs;

public:
    TypeDecl* const VoidType = nullptr;
    
    TypeDecl* const BoolType = nullptr;
    VEC_TYPES(Bool);
    MATRIX_TYPES(Bool);

    TypeDecl* const HalfType = nullptr;
    TypeDecl* const FloatType = nullptr;
    VEC_TYPES(Float);
    MATRIX_TYPES(Float);

    TypeDecl* const UIntType = nullptr;
    VEC_TYPES(UInt);
    MATRIX_TYPES(UInt);
    
    TypeDecl* const IntType = nullptr;
    VEC_TYPES(Int);
    MATRIX_TYPES(Int);
    
    TypeDecl* const DoubleType = nullptr;
    TypeDecl* const U64Type = nullptr;
    TypeDecl* const I64Type = nullptr;
};

#undef VEC_TYPES
#undef MATRIX_TYPES

} // namespace skr::SSL