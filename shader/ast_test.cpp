#include <iostream>
#include <cassert>
#include <codecvt>
#include "SSL/Decl.hpp"
#include "SSL/AST.hpp"
#include "SSL/TestASTVisitor.hpp"

int main()
{
    using namespace skr::SSL;
    AST AST = {};
    auto fields = std::vector<FieldDecl*>();
    fields.emplace_back(AST.DeclareField(L"i", AST.IntType));
    auto DataType = AST.DeclareType(L"Data", fields);

    std::vector<Expr*> inits = {
        AST.Constant(L"0.f"),
        AST.Constant(L"1.f"),
        AST.Constant(L"2.f"),
        AST.Constant(L"3.f")
    };
    auto a = AST.Variable(AST.Float4Type, L"a", AST.InitList(inits));
    auto b = AST.Variable(AST.FloatType, L"b");
    auto c = AST.Variable(AST.Float4Type, L"c");
    auto init_a = AST.Assign(a->ref(), AST.Constant(L"3.5f"));
    auto init_b = AST.Assign(b->ref(), AST.Constant(L"5.5f"));
    auto init_c = AST.Assign(c->ref(), AST.Add(a->ref(), b->ref()));
    
    auto data = AST.Variable(DataType, L"data", AST.InitList({}));
    auto test_field = AST.Assign(AST.Member(data->ref(), fields[0]), AST.Constant(L"2"));

    auto block = AST.Block({ a, b, c, init_a, init_b, init_c, data, test_field });
    auto func = AST.DeclareFunction(L"main", AST.IntType, {}, block);

    ASTVisitor visitor = {};
    String content = L"";
    for (auto type : AST.types())
        content += visitor.visit(type);
    for (auto func : AST.funcs())
        content += visitor.visit(func);
    std::wcout << content << std::endl;

    return 0;
}