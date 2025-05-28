#include <iostream>
#include <cassert>
#include "SSL/Constant.hpp"
#include "SSL/Decl.hpp"
#include "SSL/AST.hpp"
#include "SSL/TestASTVisitor.hpp"

void mandelbrot(skr::SSL::AST& AST)
{
    using namespace skr::SSL;
    std::vector<ParamVarDecl*> cos_params = { AST.DeclareParam(AST.Float3Type, L"v") };
    auto cos_func = AST.DeclareFunction(L"cos", AST.Float3Type, cos_params, nullptr);
    std::vector<ParamVarDecl*> dot_params = { AST.DeclareParam(AST.Float2Type, L"a"), AST.DeclareParam(AST.Float2Type, L"b") };
    auto dot_func = AST.DeclareFunction(L"dot", AST.FloatType, dot_params, nullptr);

    using namespace skr::SSL;
    auto tid = AST.DeclareParam(AST.UInt2Type, L"tid");
    auto tsize = AST.DeclareParam(AST.UInt2Type, L"tsize");
    std::vector<ParamVarDecl*> mandelbrot_params = { tid, tsize };
    auto mandelbrot_body = AST.Block({});
    auto mandelbrot = AST.DeclareFunction(L"mandelbrot", AST.Float4Type, mandelbrot_params, mandelbrot_body);
    
    // const float x = float(tid.x) / (float)tsize.x;
    auto x = AST.Variable(AST.FloatType, L"x", 
        AST.Div(
            AST.Field(tid->ref(), AST.UInt2Type->get_field(L"x")), 
            AST.Field(tsize->ref(), AST.UInt2Type->get_field(L"x"))
        ));
    mandelbrot_body->add_statement(x);
    
    // const float y = float(tid.y) / (float)tsize.y;
    auto y = AST.Variable(AST.FloatType, L"y",
        AST.Div(
            AST.Field(tid->ref(), AST.UInt2Type->get_field(L"y")), 
            AST.Field(tsize->ref(), AST.UInt2Type->get_field(L"y"))
        ));
    mandelbrot_body->add_statement(y);

    // const float2 uv = float2(x, y);
    std::vector<Expr*> uv_inits = { x->ref(), y->ref() }; 
    auto uv = AST.Variable(AST.Float2Type, L"uv", AST.Construct(AST.Float2Type, uv_inits));
    mandelbrot_body->add_statement(uv);

    // float n = 0.0f;
    auto n = AST.Variable(AST.FloatType, L"n", AST.Constant(FloatValue("0.0f")));
    mandelbrot_body->add_statement(n);

    // float2 c = float2(-0.444999992847442626953125f, 0.0f);
    std::vector<Expr*> c_inits = {
        AST.Constant(FloatValue("-0.444999992847442626953125f")),
        AST.Constant(FloatValue("0.0f"))
    };
    auto c = AST.Variable(AST.Float2Type, L"c", AST.Construct(AST.Float2Type, c_inits));
    mandelbrot_body->add_statement(c);

    // c = c + (uv - float2(0.5f, 0.5f)) * 2.3399999141693115234375f;
    auto vec_05_05_init = std::vector<Expr*>{ 
        AST.Constant(FloatValue("0.5f")), 
        AST.Constant(FloatValue("0.5f")) 
    };
    auto modify_c = AST.Assign(c->ref(), 
    AST.Add(c->ref(), AST.Mul(
        AST.Sub(uv->ref(), AST.Construct(AST.Float2Type, vec_05_05_init)), 
        AST.Constant(FloatValue("2.3399999141693115234375f"))
    )));
    mandelbrot_body->add_statement(modify_c);

    // float2 z = float2(0.f, 0.f);
    std::vector<Expr*> z_inits = {
        AST.Constant(FloatValue("0.f")),
        AST.Constant(FloatValue("0.f"))
    };
    auto z = AST.Variable(AST.Float2Type, L"z", AST.InitList(z_inits));
    mandelbrot_body->add_statement(z);

    // const int M = 128;
    auto M = AST.Variable(AST.IntType, L"M", AST.Constant(IntValue(128)));
    mandelbrot_body->add_statement(M);

    // TODO: FOR LOOP
    /*
        for (int i = 0; i < M; i++) {
            z = float2((z.x * z.x) - (z.y * z.y), (2.0f * z.x) * z.y) + c;
            if (dot(z, z) > 2.0f) {
                break;
            }
            n += 1.0f;
        }
    */
    auto for_init = AST.Variable(AST.IntType, L"i", AST.Constant(IntValue(0)));
    auto for_cond = AST.Less(for_init->ref(), M->ref());
    auto for_inc = AST.AddAssign(for_init->ref(), AST.Constant(IntValue(1)));
    auto for_body = AST.Block({});
    
    // z = float2((z.x * z.x) - (z.y * z.y), (2.0f * z.x) * z.y) + c;
    {
        auto temp_a = AST.Mul(AST.Field(z->ref(), AST.Float2Type->get_field(L"x")), AST.Field(z->ref(), AST.Float2Type->get_field(L"x")));
        auto temp_b = AST.Mul(AST.Field(z->ref(), AST.Float2Type->get_field(L"y")), AST.Field(z->ref(), AST.Float2Type->get_field(L"y")));
        auto temp_c = AST.Sub(temp_a, temp_b);

        auto temp_d = AST.Mul(AST.Constant(FloatValue("2.0f")), AST.Field(z->ref(), AST.Float2Type->get_field(L"x")));
        auto temp_e = AST.Mul(temp_d, AST.Field(z->ref(), AST.Float2Type->get_field(L"y")));
        std::vector<Expr*> z_inits_for = { temp_c, temp_e };
        auto temp_f = AST.Construct(AST.Float2Type, z_inits_for);
        auto modify_z = AST.Assign(z->ref(), AST.Add(temp_f, c->ref()));
        for_body->add_statement(modify_z);
    }

    // if (dot(z, z) > 2.0f) break;
    {
        std::vector<Expr*> dot_args = { z->ref(), z->ref() };
        auto temp_a = AST.CallFunction(dot_func->ref(), dot_args);
        auto if_stmt = AST.If(AST.Greater(temp_a, AST.Constant(FloatValue("2.0f"))), AST.Block({ AST.Break() }));
        for_body->add_statement(if_stmt);
    }

    // n += 1.0f;
    auto increment_n = AST.AddAssign(n->ref(), AST.Constant(FloatValue("1.0f")));
    for_body->add_statement(increment_n);

    mandelbrot_body->add_statement(AST.For(for_init, for_cond, for_inc, for_body));

    // const float t = float(n) / float(M);
    auto t = AST.Variable(AST.FloatType, L"t", AST.Div(n->ref(), AST.StaticCast(AST.FloatType, M->ref())));
    mandelbrot_body->add_statement(t);

    // const float3 d = float3(0.3f, 0.3f, 0.5f);
    std::vector<Expr*> d_inits = {
        AST.Constant(FloatValue("0.3f")),
        AST.Constant(FloatValue("0.3f")),
        AST.Constant(FloatValue("0.5f"))
    };
    auto d = AST.Variable(AST.Float3Type, L"d", AST.Construct(AST.Float3Type, d_inits));
    mandelbrot_body->add_statement(d);
    
    // const float3 e = float3(-0.2f, -0.3f, -0.5f);
    std::vector<Expr*> e_inits = {
        AST.Constant(FloatValue("-0.2f")),
        AST.Constant(FloatValue("-0.3f")),
        AST.Constant(FloatValue("-0.5f"))
    };
    auto e = AST.Variable(AST.Float3Type, L"e", AST.Construct(AST.Float3Type, e_inits));
    mandelbrot_body->add_statement(e);

    // const float3 f = float3(2.1f, 2.0f, 3.0f);
    std::vector<Expr*> f_inits = {
        AST.Constant(FloatValue("2.1f")),
        AST.Constant(FloatValue("2.0f")),
        AST.Constant(FloatValue("3.0f"))
    };
    auto f = AST.Variable(AST.Float3Type, L"f", AST.Construct(AST.Float3Type, f_inits));
    mandelbrot_body->add_statement(f);

    // const float3 g = float3(0.0f, 0.1f, 0.0f);
    std::vector<Expr*> g_inits = {
        AST.Constant(FloatValue("0.0f")),
        AST.Constant(FloatValue("0.1f")),
        AST.Constant(FloatValue("0.0f"))
    };
    auto g = AST.Variable(AST.Float3Type, L"g", AST.Construct(AST.Float3Type, g_inits));
    mandelbrot_body->add_statement(g);

    // return float4(d + (e * cos(((f * t) + g) * 2.f * pi)), 1.0f);
    auto temp0 = AST.Add(AST.Mul(f->ref(), t->ref()), g->ref());
    Expr* temp1 = AST.Mul(AST.Mul(temp0, AST.Constant(FloatValue("2.f"))), AST.Constant(FloatValue("3.14159265358979323846f"))); // pi
    auto temp2 = AST.CallFunction(cos_func->ref(), std::span<Expr*>(&temp1, 1));
    auto temp3 = AST.Mul(e->ref(), temp2);
    auto temp4 = AST.Add(d->ref(), temp3);
    std::vector<Expr*> return_inits = { temp4, AST.Constant(FloatValue("1.0f")) };
    auto return_value = AST.Construct(AST.Float4Type, return_inits);
    mandelbrot_body->add_statement(AST.Return(return_value));
}

void some_test(skr::SSL::AST& AST)
{
    using namespace skr::SSL;
    auto fields = std::vector<FieldDecl*>();
    fields.emplace_back(AST.DeclareField(L"i", AST.IntType));
    auto DataType = AST.DeclareType(L"Data", fields);
    auto Method = AST.DeclareMethod(DataType, L"do_something", AST.VoidType, {}, nullptr);
    DataType->add_method(Method);

    auto param = AST.DeclareParam(AST.Float4Type, L"param");
    auto param_ref = (Expr*)param->ref();
    std::vector<Expr*> inits = {
        AST.StaticCast(AST.FloatType, AST.Constant(FloatValue("0.f"))),
        AST.ImplicitCast(AST.FloatType, AST.Constant(FloatValue("1.f"))),
        AST.BitwiseCast(AST.FloatType, AST.Constant(FloatValue("2.f"))),
        AST.Unary(UnaryOp::PLUS, AST.Constant(FloatValue("3.f")))
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
        AST.Field(d->ref(), AST.Float4Type->get_field(L"x")),
        AST.Field(d->ref(), AST.Float4Type->get_field(L"y"))
    );
    
    auto data = AST.Variable(DataType, L"data", AST.InitList({}));
    auto test_field = AST.Assign(AST.Field(data->ref(), fields[0]), AST.Constant(IntValue(2)));

    auto block = AST.Block({ a, b, c, init_a, init_b, init_c, d, init_d, modify_d, data, test_field });
    auto func = AST.DeclareFunction(L"main", AST.IntType, std::span(&param, 1), block);

    Expr* construct = AST.Construct(AST.Float4Type, inits);
    block->add_statement(AST.CallMethod(AST.Method(data->ref(), Method), {}));
    block->add_statement(AST.CallFunction(func->ref(), std::span<Expr*>(&construct, 1)));
}

int main()
{
    using namespace skr::SSL;
    AST AST = {};

    // some_test(AST);
    mandelbrot(AST);

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