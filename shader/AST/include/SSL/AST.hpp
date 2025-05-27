#pragma once
#include "Expr.hpp"
#include "Decl.hpp"
#include "Constant.hpp"
#include <map>

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

    BinaryExpr* Add(Expr* left, Expr* right);
    BinaryExpr* Assign(Expr* left, Expr* right);
    CompoundStmt* Block(const std::vector<Stmt*>& statements);
    CallExpr* Call(DeclRefExpr* callee, std::span<Expr*> args);
    ConstantExpr* Constant(const IntValue& v);
    ConstantExpr* Constant(const FloatValue& v);
    ConstructExpr* Construct(const TypeDecl* type, std::span<Expr*> args);
    MemberExpr* Member(DeclRefExpr* base, const FieldDecl* field);
    DeclRefExpr* Ref(const Decl* decl);
    DeclStmt* Variable(const TypeDecl* type, Expr* initializer = nullptr);
    DeclStmt* Variable(const TypeDecl* type, const Name& name, Expr* initializer = nullptr);
    InitListExpr* InitList(std::span<Expr*> exprs);

    TypeDecl* const DeclareType(const Name& name, std::span<FieldDecl*> members);
    TypeDecl* const DeclarePrimitiveType(const Name& name, uint32_t size, uint32_t alignment = 4, std::vector<FieldDecl*> fields = {});
    ArrayTypeDecl* const DeclareArrayType(TypeDecl* const element, uint32_t count);
    FieldDecl* DeclareField(const Name& name, const TypeDecl* type);
    FunctionDecl* DeclareFunction(const Name& name, TypeDecl* const return_type, std::span<ParamVarDecl* const> params, CompoundStmt* body);
    ParamVarDecl* DeclareParam(const TypeDecl* type, const Name& name);

    const TypeDecl* GetType(const Name& name) const;

    std::span<Decl* const> decls() const { return _decls; }
    std::span<Stmt* const> stmts() const { return _stmts; }
    std::span<TypeDecl* const> types() const { return _types; }
    std::span<FunctionDecl* const> funcs() const { return _funcs; }

    String dump() const;

private:
    std::vector<Decl*> _decls;
    std::vector<Stmt*> _stmts;
    std::vector<TypeDecl*> _types;
    std::vector<FunctionDecl*> _funcs;
    std::map<std::pair<TypeDecl*, uint32_t>, ArrayTypeDecl*> _arrs;

public:
    TypeDecl* const VoidType = nullptr;
    
    TypeDecl* const BoolType = nullptr;
    VEC_TYPES(Bool);
    MATRIX_TYPES(Bool);

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