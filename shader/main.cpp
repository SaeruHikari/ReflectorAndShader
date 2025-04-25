#include <iostream>
#include <cassert>
#include "Type.hpp"
#include "Expression.hpp"

struct SourceBuilder
{
public:
    SourceBuilder& append(const std::u8string& content)
    {
        _source += content;
        return *this;
    }

    SourceBuilder& endline()
    {
        _source += u8"\n";
        return *this;
    }

    SourceBuilder& endline(char8_t sep)
    {
        _source += sep;
        _source += u8"\n";
        return *this;
    }

    const std::u8string& content() const { return _source; }

private:
    std::u8string _source = u8"";
};

struct ASTVisitor
{
    skr::shader::ast::Name visit(const skr::shader::ast::Expression* expr)
    {
        using namespace skr::shader::ast;
        SourceBuilder sb = {};
        if (auto binary = dynamic_cast<const BinaryExpression*>(expr))
        {
            const auto left = visit(binary->left());
            const auto right = visit(binary->right());
            auto op = binary->op();
            Name op_name = u8"";
            switch (op)
            {
            case BinaryOp::ADD:
                op_name = u8" + ";
                break;
            case BinaryOp::SUB:
                op_name = u8" - ";
                break;
            case BinaryOp::MUL:
                op_name = u8" * ";
                break;
            case BinaryOp::DIV:
                op_name = u8" / ";
                break;
            case BinaryOp::MOD:
                op_name = u8" % ";
                break;

            case BinaryOp::BIT_AND:
                op_name = u8" & ";
                break;
            case BinaryOp::BIT_OR:
                op_name = u8" | ";
                break;
            case BinaryOp::BIT_XOR:
                op_name = u8" ^ ";
                break;
            case BinaryOp::SHL:
                op_name = u8" << ";
                break;
            case BinaryOp::SHR:
                op_name = u8" >> ";
                break;
            case BinaryOp::AND:
                op_name = u8" && ";
                break;
            case BinaryOp::OR:
                op_name = u8" || ";
                break;

            case BinaryOp::ASSIGN:
                op_name = u8" = ";
                break;
            case BinaryOp::ADD_ASSIGN:
                op_name = u8" += ";
                break;
            case BinaryOp::SUB_ASSIGN:
                op_name = u8" -= ";
                break;
            case BinaryOp::MUL_ASSIGN:
                op_name = u8" *= ";
                break;
            case BinaryOp::DIV_ASSIGN:
                op_name = u8" /= ";
                break;
            case BinaryOp::MOD_ASSIGN:
                op_name = u8" %= ";
                break;
            default:
                assert(false && "Unsupported binary operation");
            }
            sb.append(left + op_name + right);
            if (((uint32_t)binary->op() & (uint32_t)BinaryOp::ASSIGN) != 0)
                sb.endline(u8';');
        }
        else if (auto param = dynamic_cast<const ParameterExpression*>(expr))
        {
            sb.append(param->name());
        }
        else if (auto constant = dynamic_cast<const ConstantExpression*>(expr))
        {
            sb.append(constant->v);
        }
        else if (auto block = dynamic_cast<const BlockExpression*>(expr))
        {
            sb.endline(u8'{');
            {
                for (auto var : block->variables())
                {
                    sb.append(var->type().name() + u8" " + var->name());
                    sb.endline(u8';');
                }
                for (auto expr : block->expressions())
                {
                    sb.append(visit(expr));
                }
            }
            sb.endline(u8'}');
        }
        return sb.content();
    }
};

int main()
{
    using namespace skr::shader::ast;
    TypeFactory types = {};
    types.add_type(Type(u8"float", sizeof(float)));
    types.add_type(Type(u8"uint32_t", sizeof(uint32_t)));
    types.add_type(Type(u8"uint64_t", sizeof(uint64_t)));
    types.add_type(Type(u8"int32_t", sizeof(int32_t)));
    types.add_type(Type(u8"int64_t", sizeof(int64_t)));

    ExpressionFactory Expression = {};
    auto a = Expression.Variable(types.get_type(u8"float"), u8"a");
    auto b = Expression.Variable(types.get_type(u8"float"), u8"b");
    auto c = Expression.Variable(types.get_type(u8"float"), u8"c");

    auto block = Expression.Block();
    block->add_variables(a, b, c);
    block->add_expressions(
        Expression.Assign(a, Expression.Constant(u8"3.5f")),
        Expression.Assign(b, Expression.Constant(u8"5.5f")),
        Expression.Assign(c, Expression.Add(a, b))
    );

    ASTVisitor visitor = {};
    Name content = visitor.visit(block);
    std::cout << std::string((const char*)content.c_str()) << std::endl;

    return 0;
}