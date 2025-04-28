#pragma once
#include "Expr.hpp"
#include "Decl.hpp"

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
    DeclRefExpr* Ref(const DeclStmt* decl);
    DeclStmt* Variable(const TypeDecl* type, Expr* initializer = nullptr);
    DeclStmt* Variable(const TypeDecl* type, const Name& name, Expr* initializer = nullptr);
    FieldDecl* Field(const Name& name, const TypeDecl* type);
    FunctionDecl* Function(const Name& name, CompoundStmt* body);

    TypeDecl* const AddType(const Name& name, std::span<FieldDecl*> members);
    TypeDecl* const AddBuiltinType(const Name& name, uint32_t size, uint32_t alignment = 4);
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

public:
    TypeDecl* const VoidType = nullptr;
    TypeDecl* const F32Type = nullptr;
    TypeDecl* const U32Type = nullptr;
    TypeDecl* const U64Type = nullptr;
    TypeDecl* const I32Type = nullptr;
    TypeDecl* const I64Type = nullptr;
};
    
} // namespace skr::SSL