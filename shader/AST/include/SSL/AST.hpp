#pragma once
#include "Expr.hpp"
#include "Decl.hpp"
#include <map>

namespace skr::SSL {

struct AST
{
public:
    AST();
    ~AST();

    BinaryExpr* Add(Expr* left, Expr* right);
    BinaryExpr* Assign(Expr* left, Expr* right);
    CompoundStmt* Block(const std::vector<Stmt*>& statements);
    ConstantExpr* Constant(const FloatSemantics& v);
    TypeDecl* const DeclareType(const Name& name, std::span<FieldDecl*> members);
    TypeDecl* const DeclarePrimitiveType(const Name& name, uint32_t size, uint32_t alignment = 4);
    ArrayTypeDecl* const DeclareArrayType(TypeDecl* const element, uint32_t count);
    DeclRefExpr* Ref(const DeclStmt* decl);
    DeclStmt* Variable(const TypeDecl* type, Expr* initializer = nullptr);
    DeclStmt* Variable(const TypeDecl* type, const Name& name, Expr* initializer = nullptr);
    FieldDecl* Field(const Name& name, const TypeDecl* type);
    FunctionDecl* Function(const Name& name, TypeDecl* const return_type, std::span<ParamVarDecl* const> params, CompoundStmt* body);
    ParamVarDecl* Param(const TypeDecl* type, const Name& name);


    const TypeDecl* GetType(const Name& name) const;

    std::span<Decl* const> decls() const { return _decls; }
    std::span<Stmt* const> stmts() const { return _stmts; }
    std::span<TypeDecl* const> types() const { return _types; }
    std::span<FunctionDecl* const> funcs() const { return _funcs; }

private:
    std::vector<Decl*> _decls;
    std::vector<Stmt*> _stmts;
    std::vector<TypeDecl*> _types;
    std::vector<FunctionDecl*> _funcs;
    std::map<std::pair<TypeDecl*, uint32_t>, ArrayTypeDecl*> _arrs;

public:
#define VEC_TYPES(N) TypeDecl* const N##2Type = nullptr; TypeDecl* const N##3Type = nullptr; TypeDecl* const N##4Type = nullptr;
#define MATRIX_TYPES(N) \
    TypeDecl* const N##1x1Type = nullptr; TypeDecl* const N##1x2Type = nullptr; TypeDecl* const N##1x3Type = nullptr; TypeDecl* const N##1x4Type = nullptr;\
    TypeDecl* const N##2x1Type = nullptr; TypeDecl* const N##2x2Type = nullptr; TypeDecl* const N##2x3Type = nullptr; TypeDecl* const N##2x4Type = nullptr;\
    TypeDecl* const N##3x1Type = nullptr; TypeDecl* const N##3x2Type = nullptr; TypeDecl* const N##3x3Type = nullptr; TypeDecl* const N##3x4Type = nullptr;\
    TypeDecl* const N##4x1Type = nullptr; TypeDecl* const N##4x2Type = nullptr; TypeDecl* const N##4x3Type = nullptr; TypeDecl* const N##4x4Type = nullptr;

    TypeDecl* const VoidType = nullptr;
    TypeDecl* const FloatType = nullptr;
    VEC_TYPES(Float);
    MATRIX_TYPES(Float);

    TypeDecl* const UIntType = nullptr;
    VEC_TYPES(UInt);
    MATRIX_TYPES(UInt);
    
    TypeDecl* const IntType = nullptr;
    VEC_TYPES(Int);
    MATRIX_TYPES(Int);
    
    TypeDecl* const U64Type = nullptr;
    TypeDecl* const I64Type = nullptr;
};

} // namespace skr::SSL