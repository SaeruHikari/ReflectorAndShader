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
    fields.emplace_back(AST.Field(L"i", AST.IntType));
    auto DataType = AST.DeclareType(L"Data", fields);

    auto a = AST.Variable(AST.FloatType, L"a", AST.Constant(L"2.f"));
    auto b = AST.Variable(AST.FloatType, L"b");
    auto c = AST.Variable(AST.FloatType, L"c");
    auto init_a = AST.Assign(a->ref(), AST.Constant(L"3.5f"));
    auto init_b = AST.Assign(b->ref(), AST.Constant(L"5.5f"));
    auto init_c = AST.Assign(c->ref(), AST.Add(a->ref(), b->ref()));

    auto block = AST.Block({ a, b, c, init_a, init_b, init_c });
    auto func = AST.Function(L"main", AST.IntType, {}, block);

    ASTVisitor visitor = {};
    String content = L"";
    for (auto type : AST.types())
        content += visitor.visit(type);
    for (auto func : AST.funcs())
        content += visitor.visit(func);
    std::wcout << content << std::endl;

    return 0;
}