#include "SSL/ASTDumper.hpp"
#include <typeinfo>

namespace skr::SSL {

void ASTDumper::visit(const skr::SSL::Stmt* stmt, SourceBuilderNew& sb)
{
    using namespace skr::SSL;
    if (auto init = dynamic_cast<const InitListExpr*>(stmt))
    {
        sb.append_expr(L"InitListExpr ");
        sb.endline();
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

        sb.append_expr(L"BinaryExpr ");
        sb.append(op_name);
        sb.endline();
        
        sb.indent([&](){
            visit(binary->left(), sb);
            Visit(binary->right(), sb);
        });
    }
    else if (auto callExpr = dynamic_cast<const CallExpr*>(stmt))
    {
        sb.append_expr(L"CallExpr");
        sb.endline();
    }
    else if (auto constructExpr = dynamic_cast<const ConstructExpr*>(stmt))
    {
        sb.append_expr(L"ConstructExpr ");
        sb.append_type(constructExpr->type()->name());
        sb.endline();
    }
    else if (auto declStmt = dynamic_cast<const DeclStmt*>(stmt))
    {
        sb.append_expr(L"DeclStmt ");
        sb.endline();
        
        sb.indent([&](){
            if (auto decl = dynamic_cast<const VarDecl*>(declStmt->decl()))
            {
                visit(decl, sb);
            }
        });
    }
    else if (auto declRef = dynamic_cast<const DeclRefExpr*>(stmt))
    {
        sb.append_expr(L"DeclRefExpr ");
        
        if (auto var = dynamic_cast<const VarDecl*>(declRef->decl()))
            sb.append(L"var " + var->name());
        else if (auto func = dynamic_cast<const FunctionDecl*>(declRef->decl()))
            sb.append(L"function " + func->name());
        else if (auto type = dynamic_cast<const TypeDecl*>(declRef->decl()))
            sb.append(L"type " + type->name());
        else
            sb.append(L"UnknownDecl");

        sb.endline();
    }
    else if (auto constant = dynamic_cast<const ConstantExpr*>(stmt))
    {
        sb.append_expr(L"ConstantExpr ");
        if (auto i = std::get_if<IntValue>(&constant->value))
        {
            if (i->is_signed())
                sb.append(L"IntValue: " + std::to_wstring(i->value<int64_t>().get()));
            else
                sb.append(L"UIntValue: " + std::to_wstring(i->value<uint64_t>().get()));
        }
        else if (auto f = std::get_if<FloatValue>(&constant->value))
        {
            sb.append(L"FloatValue: " + std::to_wstring(f->ieee.value()));
        }
        else
        {
            sb.append(L"UnknownConstant: ");
        }
        sb.endline();
    }
    else if (auto member = dynamic_cast<const MemberExpr*>(stmt))
    {
        sb.append_expr(L"MemberExpr ");
        auto field = member->member_decl();
        if (auto _as_field = dynamic_cast<const FieldDecl*>(field))
        {
            sb.append(_as_field->name());
        }
        sb.endline();

        sb.indent([&](){
            auto owner = member->owner();
            visit(owner, sb);
        });
    }
    else if (auto block = dynamic_cast<const CompoundStmt*>(stmt))
    {
        sb.append_expr(L"CompoundStmt ");
        sb.endline();
    }
    else
    {
        sb.append_expr(L"UnknownExpr ");
        sb.endline();

    }
    
    sb.indent([&](){
        for (auto expr : stmt->children())
        {
            visit(expr, sb);
            sb.endline();
        }
    });
}

void ASTDumper::visit(const skr::SSL::TypeDecl* typeDecl, SourceBuilderNew& sb)
{
    using namespace skr::SSL;
    if (typeDecl->is_builtin())
    {
        sb.append_decl(L"BuiltinType ");
        sb.append_type(typeDecl->name());
        sb.endline();
    }
    else
    {
        sb.append_decl(L"TypeDecl ");
        sb.append_type(typeDecl->name());
        sb.endline();
        // sb.indent();
        for (auto field : typeDecl->fields())
        {
            visit(field, sb);
        }
    }
}

void ASTDumper::visit(const skr::SSL::FieldDecl* fieldDecl, SourceBuilderNew& sb)
{
    using namespace skr::SSL;
    sb.append_decl(L"FieldDecl ");
    sb.append(fieldDecl->name());
    sb.append_type(L" '" + fieldDecl->type().name() + L"'");
    sb.endline();
}

void ASTDumper::visit(const skr::SSL::ParamVarDecl* paramDecl, SourceBuilderNew& sb)
{
    using namespace skr::SSL;
    sb.append_decl(L"ParamVarDecl ");
    sb.append_type(paramDecl->type().name() + L" ");
    sb.append(paramDecl->name());
    sb.endline();
}

void ASTDumper::visit(const skr::SSL::FunctionDecl* funcDecl, SourceBuilderNew& sb)
{
    using namespace skr::SSL;
    sb.append_decl(L"FunctionDecl ");
    sb.append(funcDecl->name());
    sb.endline();
    
    sb.indent([&](){
        for (auto param : funcDecl->parameters())
        {
            visit(param, sb);
        }
        
        visit(funcDecl->body(), sb);
    });
}

void ASTDumper::visit(const skr::SSL::VarDecl* varDecl, SourceBuilderNew& sb)
{
    using namespace skr::SSL;
    sb.append_decl(L"VarDecl ");
    sb.append(varDecl->name());
    sb.endline();
    
    if (auto init = varDecl->initializer())
    {
        visit(init, sb);
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
    visit(stmt, sb);
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