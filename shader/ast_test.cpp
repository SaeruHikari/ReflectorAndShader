#include <iostream>
#include <cassert>
#include <codecvt>
#include "SSL/Decl.hpp"
#include "SSL/AST.hpp"

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
    skr::SSL::String visitExpr(const skr::SSL::Stmt* stmt)
    {
        using namespace skr::SSL;
        SourceBuilder sb = {};
        if (auto binary = dynamic_cast<const BinaryExpr*>(stmt))
        {
            const auto left = visitExpr(binary->left());
            const auto right = visitExpr(binary->right());
            auto op = binary->op();
            String op_name = u8"";
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
        else if (auto declStmt = dynamic_cast<const DeclStmt*>(stmt))
        {
            if (auto decl = dynamic_cast<const VarDecl*>(declStmt->decl()))
            {
                sb.append(decl->type().name() + u8" " + decl->name());
                if (auto init = decl->initializer())
                    sb.append(u8" = " + visitExpr(init));
                sb.endline(u8';');
            }
        }
        else if (auto declRef = dynamic_cast<const DeclRefExpr*>(stmt))
        {
            if (auto decl = dynamic_cast<const VarDecl*>(declRef->decl()))
                sb.append(decl->name());
        }
        else if (auto constant = dynamic_cast<const ConstantExpr*>(stmt))
        {
            sb.append(constant->v);
        }
        else if (auto block = dynamic_cast<const CompoundStmt*>(stmt))
        {
            sb.endline(u8'{');
            {
                for (auto expr : block->children())
                {
                    sb.append(visitExpr(expr));
                }
            }
            sb.endline(u8'}');
        }
        return sb.content();
    }

    skr::SSL::String visit(const skr::SSL::TypeDecl* typeDecl)
    {
        using namespace skr::SSL;
        SourceBuilder sb = {};
        if (typeDecl->is_builtin())
        {
            sb.append(u8"//builtin type: ");
            sb.append(typeDecl->name());
            auto sizeString = std::to_wstring(typeDecl->size());
            sb.append(u8", size: " + std::u8string((const char8_t*)sizeString.c_str()));
            sb.endline();
        }
        else
        {
            sb.append(u8"struct " + typeDecl->name());
            sb.endline(u8'{');
            for (auto field : typeDecl->fields())
            {
                sb.append(u8"    " + field->type().name() + u8" " + field->name() + u8";\n");
            }
            sb.append(u8"};\n");
        }
        return sb.content();
    }

    skr::SSL::String visit(const skr::SSL::FunctionDecl* funcDecl)
    {
        using namespace skr::SSL;
        SourceBuilder sb = {};
        sb.append(funcDecl->return_type()->name() + u8" " + funcDecl->name() + u8"(");
        /*
        for (auto param : funcDecl->parameters())
        {
            sb.append(param->type().name() + u8" " + param->name() + u8", ");
        }
        */
        sb.endline(u8')');
        sb.append(visitExpr(funcDecl->body()));
        return sb.content();
    }
};

int main()
{
    using namespace skr::SSL;
    AST AST = {};
    auto fields = std::vector<FieldDecl*>();
    fields.emplace_back(AST.Field(u8"i", AST.I32Type));
    auto DataType = AST.AddType(u8"Data", fields);

    auto a = AST.Variable(AST.F32Type, u8"a", AST.Constant(u8"2.f"));
    auto b = AST.Variable(AST.F32Type, u8"b");
    auto c = AST.Variable(AST.F32Type, u8"c");
    auto init_a = AST.Assign(a->ref(), AST.Constant(u8"3.5f"));
    auto init_b = AST.Assign(b->ref(), AST.Constant(u8"5.5f"));
    auto init_c = AST.Assign(c->ref(), AST.Add(a->ref(), b->ref()));

    auto block = AST.Block({ a, b, c, init_a, init_b, init_c });
    auto func = AST.Function(u8"main", block);

    ASTVisitor visitor = {};
    String content = u8"";
    for (auto type : AST.types())
        content += visitor.visit(type);
    for (auto func : AST.funcs())
        content += visitor.visit(func);
    std::cout << std::string((const char*)content.c_str()) << std::endl;

    return 0;
}