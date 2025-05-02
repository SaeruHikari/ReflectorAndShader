#include "SSL/ASTDumper.hpp"

namespace skr::SSL {

skr::SSL::String ASTDumper::visit(const skr::SSL::Stmt* stmt)
{
    using namespace skr::SSL;
    if (auto init = dynamic_cast<const InitListExpr*>(stmt))
    {
        sb.append(L"InitListExpr ");
        sb.endline();
        
        for (auto expr : init->children())
        {
            sb.append(visit(expr));
        }
    }
    else if (auto binary = dynamic_cast<const BinaryExpr*>(stmt))
    {
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

        sb.append(L"BinaryExpr ");
        sb.append(op_name);
        sb.endline();
        
        sb.indent([&](){
            sb.append(visit(binary->left()));
            sb.append(visit(binary->right()));
        });
    }
    else if (auto declStmt = dynamic_cast<const DeclStmt*>(stmt))
    {
        sb.append(L"DeclStmt ");
        sb.endline();

        if (auto decl = dynamic_cast<const VarDecl*>(declStmt->decl()))
        {
            sb.append(visit(decl));
        }
    }
    else if (auto declRef = dynamic_cast<const DeclRefExpr*>(stmt))
    {
        sb.append(L"DeclRefExpr ");
        if (auto decl = dynamic_cast<const VarDecl*>(declRef->decl()))
            sb.append(decl->name());
        sb.endline();
    }
    else if (auto constant = dynamic_cast<const ConstantExpr*>(stmt))
    {
        sb.append(L"ConstantExpr ");
        sb.append(constant->v);
        sb.endline();
    }
    else if (auto member = dynamic_cast<const MemberExpr*>(stmt))
    {
        sb.append(L"MemberExpr ");
        auto field = member->member_decl();
        if (auto _as_field = dynamic_cast<const FieldDecl*>(field))
        {
            sb.append(_as_field->name());
        }
        sb.endline();

        auto owner = member->owner();
        sb.append(visit(owner));
    }
    else if (auto block = dynamic_cast<const CompoundStmt*>(stmt))
    {
        sb.append(L"CompoundStmt ");
        sb.endline();
        for (auto expr : block->children())
        {
            sb.append(visit(expr));
        }
    }
    return sb.content();
}

skr::SSL::String ASTDumper::visit(const skr::SSL::TypeDecl* typeDecl)
{
    using namespace skr::SSL;
    if (typeDecl->is_builtin())
    {
        sb.append(L"BuiltinType ");
        sb.append(typeDecl->name());
        sb.endline();
    }
    else
    {
        sb.append(L"TypeDecl ");
        sb.append(typeDecl->name());
        sb.endline();
        // sb.indent();
        for (auto field : typeDecl->fields())
        {
            sb.append(visit(field));
        }
    }        
    return sb.content();
}

skr::SSL::String ASTDumper::visit(const skr::SSL::FieldDecl* fieldDecl)
{
    using namespace skr::SSL;
    sb.append(L"FieldDecl ");
    sb.append(fieldDecl->type().name() + L" " + fieldDecl->name());
    sb.endline(u8';');
    return sb.content();
}

skr::SSL::String ASTDumper::visit(const skr::SSL::ParamVarDecl* paramDecl)
{
    using namespace skr::SSL;
    sb.append(L"ParamVarDecl ");
    sb.append(paramDecl->type().name() + L" " + paramDecl->name());
    sb.endline(u8';');
    return sb.content();
}

skr::SSL::String ASTDumper::visit(const skr::SSL::FunctionDecl* funcDecl)
{
    using namespace skr::SSL;
    sb.append(L"FunctionDecl ");
    sb.append(funcDecl->name());
    sb.endline();
    for (auto param : funcDecl->parameters())
    {
        sb.append(visit(param));
    }
    sb.append(visit(funcDecl->body()));
    return sb.content();
}

skr::SSL::String ASTDumper::visit(const skr::SSL::VarDecl* varDecl)
{
    using namespace skr::SSL;
    sb.append(L"VarDecl ");
    sb.append(varDecl->name());
    sb.endline();
    if (auto init = varDecl->initializer())
        sb.append(visit(init));
    return sb.content();
}

skr::SSL::String ASTDumper::Visit(const skr::SSL::Decl* decl)
{
    if (auto typeDecl = dynamic_cast<const TypeDecl*>(decl))
    {
        return visit(typeDecl);
    }
    else if (auto fieldDecl = dynamic_cast<const FieldDecl*>(decl))
    {
        return visit(fieldDecl);
    }
    else if (auto paramDecl = dynamic_cast<const ParamVarDecl*>(decl))
    {
        return visit(paramDecl);
    }
    else if (auto funcDecl = dynamic_cast<const FunctionDecl*>(decl))
    {
        return visit(funcDecl);
    }
    else if (auto varDecl = dynamic_cast<const VarDecl*>(decl))
    {
        return visit(varDecl);
    }
    assert(0 && "Unsupported decl type");
    return L"UNSUPPORTED_DECL_TYPE";
}

skr::SSL::String ASTDumper::Visit(const skr::SSL::Stmt* stmt)
{
    return L"UNSUPPORTED_EXPR_TYPE";
}

String Decl::dump() const
{
    return ASTDumper().Visit(this);
}

String Stmt::dump() const
{
    return ASTDumper().Visit(this);
}

String AST::dump() const
{
    String content = L"";
    for (auto type : types())
    {
        content += type->dump();
    }
    for (auto func : funcs())
    {
        content += func->dump();
    }
    return content;
}

} // namespace skr::SSL