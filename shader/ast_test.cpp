#include <iostream>
#include <cassert>
#include "SSL/Constant.hpp"
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

    auto param = AST.DeclareParam(AST.Float4Type, L"param");
    auto param_ref = (Expr*)param->ref();
    std::vector<Expr*> inits = {
        AST.Constant(FloatValue("0.f")),
        AST.Constant(FloatValue("1.f")),
        AST.Constant(FloatValue("2.f")),
        AST.Constant(FloatValue("3.f"))
    };
    auto a = AST.Variable(AST.Float4Type, L"a", AST.InitList(inits));
    auto b = AST.Variable(AST.FloatType, L"b");
    auto c = AST.Variable(AST.Float4Type, L"c");
    auto init_a = AST.Assign(a->ref(), AST.Constant(FloatValue("3.5f")));
    auto init_b = AST.Assign(b->ref(), AST.Constant(FloatValue("5.5f")));
    auto init_c = AST.Assign(c->ref(), AST.Add(a->ref(), b->ref()));

    auto d = AST.Variable(AST.Float4Type, L"d");
    auto init_d = AST.Assign(d->ref(), param->ref());
    auto modify_d = AST.Assign(
        AST.Member(d->ref(), AST.Float4Type->get_field(L"x")),
        AST.Member(d->ref(), AST.Float4Type->get_field(L"y"))
    );
    
    auto data = AST.Variable(DataType, L"data", AST.InitList({}));
    auto test_field = AST.Assign(AST.Member(data->ref(), fields[0]), AST.Constant(IntValue(2)));

    auto block = AST.Block({ a, b, c, init_a, init_b, init_c, d, init_d, modify_d, data, test_field });
    auto func = AST.DeclareFunction(L"main", AST.IntType, std::span(&param, 1), block);

    Expr* construct = AST.Construct(AST.Float4Type, inits);
    block->add_statement(AST.Call(func->ref(), std::span<Expr*>(&construct, 1)));

    ASTVisitor visitor = {};
    String content = L"";
    for (auto type : AST.types())
        content += visitor.visit(type);
    for (auto func : AST.funcs())
        content += visitor.visit(func);
    std::wcout << content << std::endl;

    std::wcout << AST.dump() << std::endl;

    return 0;
}