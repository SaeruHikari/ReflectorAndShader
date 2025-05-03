#include "SSL/ASTDumper.hpp"

namespace skr::SSL {

void ASTDumper::visit(const skr::SSL::Stmt* stmt, SourceBuilderNew& sb)
{
    using namespace skr::SSL;
    if (auto init = dynamic_cast<const InitListExpr*>(stmt))
    {
        sb.append_node_type(L"InitListExpr ");
        sb.endline();
        
        sb.indent([&](){
            for (auto expr : init->children())
            {
                auto fork = sb.fork();
                visit(expr, fork);
                sb.merge(fork);
            }
        });
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

        sb.append_node_type(L"BinaryExpr ");
        sb.append(op_name);
        sb.endline();
        
        sb.indent([&](){
            auto l_fork = sb.fork();
            visit(binary->left(), l_fork);
            sb.merge(l_fork);
            
            auto r_fork = sb.fork();
            Visit(binary->right(), r_fork);
            sb.merge(r_fork);
        });
    }
    else if (auto declStmt = dynamic_cast<const DeclStmt*>(stmt))
    {
        sb.append_node_type(L"DeclStmt ");
        sb.endline();
        
        sb.indent([&](){
            if (auto decl = dynamic_cast<const VarDecl*>(declStmt->decl()))
            {
                auto fork = sb.fork();
                visit(decl, fork);
                sb.merge(fork);
            }
        });
    }
    else if (auto declRef = dynamic_cast<const DeclRefExpr*>(stmt))
    {
        sb.append_node_type(L"DeclRefExpr ");
        if (auto decl = dynamic_cast<const VarDecl*>(declRef->decl()))
            sb.append(decl->name());
        sb.endline();
    }
    else if (auto constant = dynamic_cast<const ConstantExpr*>(stmt))
    {
        sb.append_node_type(L"ConstantExpr ");
        sb.append(constant->v);
        sb.endline();
    }
    else if (auto member = dynamic_cast<const MemberExpr*>(stmt))
    {
        sb.append_node_type(L"MemberExpr ");
        auto field = member->member_decl();
        if (auto _as_field = dynamic_cast<const FieldDecl*>(field))
        {
            sb.append(_as_field->name());
        }
        sb.endline();

        auto owner = member->owner();
        {
            auto fork = sb.fork();
            visit(owner, fork);
            sb.merge(fork);
        }
    }
    else if (auto block = dynamic_cast<const CompoundStmt*>(stmt))
    {
        sb.append_node_type(L"CompoundStmt ");
        sb.endline();
        for (auto expr : block->children())
        {
            auto fork = sb.fork();
            visit(expr, fork);
            sb.merge(fork);
            sb.endline();
        }
    }
}

void ASTDumper::visit(const skr::SSL::TypeDecl* typeDecl, SourceBuilderNew& sb)
{
    using namespace skr::SSL;
    if (typeDecl->is_builtin())
    {
        sb.append_node_type(L"BuiltinType ");
        sb.append(typeDecl->name());
        sb.endline();
    }
    else
    {
        sb.append_node_type(L"TypeDecl ");
        sb.append(typeDecl->name());
        sb.endline();
        // sb.indent();
        for (auto field : typeDecl->fields())
        {
            auto fork = sb.fork();
            visit(field, fork);
            sb.merge(fork);
        }
    }
}

void ASTDumper::visit(const skr::SSL::FieldDecl* fieldDecl, SourceBuilderNew& sb)
{
    using namespace skr::SSL;
    sb.append_node_type(L"FieldDecl ");
    sb.append_type(fieldDecl->type().name() + L" ");
    sb.append(fieldDecl->name());
    sb.endline(u8';');
}

void ASTDumper::visit(const skr::SSL::ParamVarDecl* paramDecl, SourceBuilderNew& sb)
{
    using namespace skr::SSL;
    sb.append_node_type(L"ParamVarDecl ");
    sb.append_type(paramDecl->type().name() + L" ");
    sb.append(paramDecl->name());
    sb.endline(u8';');
}

void ASTDumper::visit(const skr::SSL::FunctionDecl* funcDecl, SourceBuilderNew& sb)
{
    using namespace skr::SSL;
    sb.append_node_type(L"FunctionDecl ");
    sb.append(funcDecl->name());
    sb.endline();
    for (auto param : funcDecl->parameters())
    {
        auto fork = sb.fork();
        visit(param, fork);
        sb.merge(fork);
    }
    
    {
        auto fork = sb.fork();
        visit(funcDecl->body(), fork);
        sb.merge(fork);
    }
}

void ASTDumper::visit(const skr::SSL::VarDecl* varDecl, SourceBuilderNew& sb)
{
    using namespace skr::SSL;
    sb.append_node_type(L"VarDecl ");
    sb.append(varDecl->name());
    sb.endline();
    if (auto init = varDecl->initializer())
    {
        auto fork = sb.fork();
        visit(init, fork);
        sb.merge(fork);
    }
}

void ASTDumper::Visit(const skr::SSL::Decl* decl, SourceBuilderNew& sb)
{
    if (auto typeDecl = dynamic_cast<const TypeDecl*>(decl))
    {
        visit(typeDecl, sb);
    }
    else if (auto fieldDecl = dynamic_cast<const FieldDecl*>(decl))
    {
        visit(fieldDecl, sb);
    }
    else if (auto paramDecl = dynamic_cast<const ParamVarDecl*>(decl))
    {
        visit(paramDecl, sb);
    }
    else if (auto funcDecl = dynamic_cast<const FunctionDecl*>(decl))
    {
        visit(funcDecl, sb);
    }
    else if (auto varDecl = dynamic_cast<const VarDecl*>(decl))
    {
        visit(varDecl, sb);
    }
    else
    {
        assert(0 && "Unsupported decl type");
    }
}

void ASTDumper::Visit(const skr::SSL::Stmt* stmt, SourceBuilderNew& sb)
{
    sb.append(L"UNSUPPORTED_EXPR_TYPE");
}

String Decl::dump() const
{
    SourceBuilderNew sb;
    ASTDumper().Visit(this, sb);
    return sb.build(SourceBuilderNew::line_builder_tree);
}

String Stmt::dump() const
{
    SourceBuilderNew sb;
    ASTDumper().Visit(this, sb);
    return sb.build(SourceBuilderNew::line_builder_tree);
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