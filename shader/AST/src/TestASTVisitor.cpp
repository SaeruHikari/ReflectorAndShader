#include "SSL/TestASTVisitor.hpp"

namespace skr::SSL
{
skr::SSL::String ASTVisitor::visitExpr(const skr::SSL::Stmt* stmt)
{
    using namespace skr::SSL;
    SourceBuilder sb = {};
    if (auto init = dynamic_cast<const InitListExpr*>(stmt))
    {
        sb.append(L"{ ");
        for (size_t i = 0; i < init->children().size(); i++)
        {
            auto expr = init->children()[i];
            if (i > 0)
                sb.append(L", ");
            sb.append(visitExpr(expr));
        }
        sb.append(L" }");
    }
    else if (auto binary = dynamic_cast<const BinaryExpr*>(stmt))
    {
        const auto left = visitExpr(binary->left());
        const auto right = visitExpr(binary->right());
        auto op = binary->op();
        String op_name = L"";
        switch (op)
        {
        case BinaryOp::ADD:
            op_name = L" + ";
            break;
        case BinaryOp::SUB:
            op_name = L" - ";
            break;
        case BinaryOp::MUL:
            op_name = L" * ";
            break;
        case BinaryOp::DIV:
            op_name = L" / ";
            break;
        case BinaryOp::MOD:
            op_name = L" % ";
            break;

        case BinaryOp::BIT_AND:
            op_name = L" & ";
            break;
        case BinaryOp::BIT_OR:
            op_name = L" | ";
            break;
        case BinaryOp::BIT_XOR:
            op_name = L" ^ ";
            break;
        case BinaryOp::SHL:
            op_name = L" << ";
            break;
        case BinaryOp::SHR:
            op_name = L" >> ";
            break;
        case BinaryOp::AND:
            op_name = L" && ";
            break;
        case BinaryOp::OR:
            op_name = L" || ";
            break;

        case BinaryOp::ASSIGN:
            op_name = L" = ";
            break;
        case BinaryOp::ADD_ASSIGN:
            op_name = L" += ";
            break;
        case BinaryOp::SUB_ASSIGN:
            op_name = L" -= ";
            break;
        case BinaryOp::MUL_ASSIGN:
            op_name = L" *= ";
            break;
        case BinaryOp::DIV_ASSIGN:
            op_name = L" /= ";
            break;
        case BinaryOp::MOD_ASSIGN:
            op_name = L" %= ";
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
            sb.append(decl->type().name() + L" " + decl->name());
            if (auto init = decl->initializer())
                sb.append(L" = " + visitExpr(init));
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
        if (auto i = std::get_if<IntValue>(&constant->value))
        {
            if (i->is_signed())
                sb.append(std::to_wstring(i->value<int64_t>().get()));
            else
                sb.append(std::to_wstring(i->value<uint64_t>().get()));
        }
        else if (auto f = std::get_if<FloatValue>(&constant->value))
        {
            sb.append(std::to_wstring(f->ieee.value()));
        }
        else
        {
            sb.append(L"UnknownConstant: ");
        }
    }
    else if (auto member = dynamic_cast<const MemberExpr*>(stmt))
    {
        auto owner = member->owner();
        auto field = member->member_decl();
        if (auto _as_field = dynamic_cast<const FieldDecl*>(field))
        {
            sb.append(visitExpr(owner) + L"." + _as_field->name());
        }
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

skr::SSL::String ASTVisitor::visit(const skr::SSL::TypeDecl* typeDecl)
{
    using namespace skr::SSL;
    SourceBuilder sb = {};
    if (typeDecl->is_builtin())
    {
        sb.append(L"//builtin type: ");
        sb.append(typeDecl->name());
        sb.append(L", size: " + std::to_wstring(typeDecl->size()));
        sb.endline();
    }
    else
    {
        sb.append(L"struct " + typeDecl->name());
        sb.endline(u8'{');
        for (auto field : typeDecl->fields())
        {
            sb.append(L"    " + field->type().name() + L" " + field->name() + L";\n");
        }
        sb.append(L"};\n");
    }        
    return sb.content();
}

skr::SSL::String ASTVisitor::visit(const skr::SSL::FunctionDecl* funcDecl)
{
    using namespace skr::SSL;
    SourceBuilder sb = {};
    sb.append(funcDecl->return_type()->name() + L" " + funcDecl->name() + L"(");
    for (size_t i = 0; i < funcDecl->parameters().size(); i++)
    {
        auto param = funcDecl->parameters()[i];
        String content = param->type().name() + L" " + param->name();
        if (i > 0)
            content = L", " + content;
        sb.append(content);
    }
    sb.endline(u8')');
    sb.append(visitExpr(funcDecl->body()));
    return sb.content();
}
}