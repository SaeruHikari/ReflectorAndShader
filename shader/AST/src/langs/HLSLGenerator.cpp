#include "SSL/langs/HLSLGenerator.hpp"

namespace skr::SSL
{
void HLSLGenerator::visitExpr(SourceBuilderNew& sb, const skr::SSL::Stmt* stmt)
{
    using namespace skr::SSL;
    if (auto binary = dynamic_cast<const BinaryExpr*>(stmt))
    {
        sb.append(L"(");
        visitExpr(sb, binary->left());
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

        case BinaryOp::LESS:
            op_name = L" < ";
            break;
        case BinaryOp::GREATER:
            op_name = L" > ";
            break;
        case BinaryOp::LESS_EQUAL:
            op_name = L" <= ";
            break;
        case BinaryOp::GREATER_EQUAL:
            op_name = L" >= ";
            break;
        case BinaryOp::EQUAL:
            op_name = L" == ";
            break;
        case BinaryOp::NOT_EQUAL:
            op_name = L" != ";
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
        sb.append(op_name);
        visitExpr(sb, binary->right());
        sb.append(L")");

        if (auto parent = stmt->parent())
        {
            bool IsStatement = dynamic_cast<const CompoundStmt*>(parent) ||
                               // dynamic_cast<const IfStmt*>(parent) ||
                               // dynamic_cast<const ForStmt*>(parent) ||
                               dynamic_cast<const WhileStmt*>(parent) ||
                               dynamic_cast<const SwitchStmt*>(parent);
            if (IsStatement)
                sb.endline(L';');
        }
    }
    else if (auto bitwiseCast = dynamic_cast<const BitwiseCastExpr*>(stmt))
    {
        auto _type = bitwiseCast->type();
        sb.append(L"bit_cast<" + _type->name() + L">(");
        visitExpr(sb, bitwiseCast->expr());
        sb.append(L")");
    }
    else if (auto breakStmt = dynamic_cast<const BreakStmt*>(stmt))
    {
        sb.append(L"break;");
    }
    else if (auto block = dynamic_cast<const CompoundStmt*>(stmt))
    {
        sb.endline(L'{');
        sb.indent([&](){
            for (auto expr : block->children())
            {
                visitExpr(sb, expr);
            }
        });
        sb.append(L"}");
    }
    else if (auto callExpr = dynamic_cast<const CallExpr*>(stmt))
    {
        auto callee = callExpr->callee();
        if (auto callee_decl = dynamic_cast<const FunctionDecl*>(callee->decl()))
        {
            sb.append(callee_decl->name());
            sb.append(L"(");
            for (size_t i = 0; i < callExpr->args().size(); i++)
            {
                auto arg = callExpr->args()[i];
                if (i > 0)
                    sb.append(L", ");
                visitExpr(sb, arg);
            }
            sb.append(L")");
        }
        else
        {
            sb.append(L"unknown function call!");
        }
    }
    else if (auto caseStmt = dynamic_cast<const CaseStmt*>(stmt))
    {
        if (caseStmt->cond())
        {
            sb.append(L"case ");
            visitExpr(sb, caseStmt->cond());
            sb.append(L":");
        }
        else
        {
            sb.append(L"default:");
        }
        visitExpr(sb, caseStmt->body());
    }
    else if (auto methodCall = dynamic_cast<const MethodCallExpr*>(stmt))
    {
        auto callee = methodCall->callee();
        if (callee)
            visitExpr(sb, callee);
        
        sb.append(L"(");
        for (size_t i = 0; i < methodCall->args().size(); i++)
        {
            auto arg = methodCall->args()[i];
            if (i > 0)
                sb.append(L", ");
            visitExpr(sb, arg);
        }
        sb.append(L")");
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
    else if (auto constructExpr = dynamic_cast<const ConstructExpr*>(stmt))
    {
        sb.append(constructExpr->type()->name() + L"(");
        for (size_t i = 0; i < constructExpr->args().size(); i++)
        {
            auto arg = constructExpr->args()[i];
            if (i > 0)
                sb.append(L", ");
            visitExpr(sb, arg);
        }
        sb.append(L")");
    }
    else if (auto continueStmt = dynamic_cast<const ContinueStmt*>(stmt))
    {
        sb.append(L"continue;");
    }
    else if (auto defaultStmt = dynamic_cast<const DefaultStmt*>(stmt))
    {
        sb.append(L"default:");
    }
    else if (auto member = dynamic_cast<const MemberExpr*>(stmt))
    {
        auto owner = member->owner();
        auto field = member->member_decl();
        if (auto _as_field = dynamic_cast<const FieldDecl*>(field))
        {
            visitExpr(sb, owner);
            sb.append(L"." + _as_field->name());
        }
        else if (auto _as_method = dynamic_cast<const MethodDecl*>(field))
        {
            visitExpr(sb, owner);
            sb.append(L"." + _as_method->name() + L"(");
            for (size_t i = 0; i < _as_method->parameters().size(); i++)
            {
                auto param = _as_method->parameters()[i];
                if (i > 0)
                    sb.append(L", ");
                sb.append(param->type().name() + L" " + param->name());
            }
            sb.append(L")");
        }
        else
        {
            sb.append(L"UnknownMember");
        }
    }
    else if (auto forStmt = dynamic_cast<const ForStmt*>(stmt))
    {
        sb.append(L"for (");
        if (forStmt->init())
            visitExpr(sb, forStmt->init());
        
        if (forStmt->cond())
            visitExpr(sb, forStmt->cond());
        sb.append(L"; ");

        if (forStmt->inc())
            visitExpr(sb, forStmt->inc());
        sb.append(L") ");

        visitExpr(sb, forStmt->body());
    }
    else if (auto ifStmt = dynamic_cast<const IfStmt*>(stmt))
    {
        sb.append(L"if (");
        visitExpr(sb, ifStmt->cond());
        sb.append(L") ");
        sb.indent([&](){
            visitExpr(sb, ifStmt->then_body());
        });

        if (ifStmt->else_body())
        {
            sb.append(L" else ");
            sb.indent([&](){
                visitExpr(sb, ifStmt->else_body());
            });
        }
    }
    else if (auto initList = dynamic_cast<const InitListExpr*>(stmt))
    {
        sb.append(L"{ ");
        for (size_t i = 0; i < initList->children().size(); i++)
        {
            auto expr = initList->children()[i];
            if (i > 0)
                sb.append(L", ");
            visitExpr(sb, expr);
        }
        sb.append(L" }");
    }
    else if (auto implicitCast = dynamic_cast<const ImplicitCastExpr*>(stmt))
    {
        // do nothing ...
    }
    else if (auto declRef = dynamic_cast<const DeclRefExpr*>(stmt))
    {
        if (auto decl = dynamic_cast<const VarDecl*>(declRef->decl()))
            sb.append(decl->name());
    }
    else if (auto returnStmt = dynamic_cast<const ReturnStmt*>(stmt))
    {
        sb.append(L"return");
        if (returnStmt->value())
        {
            sb.append(L" ");
            visitExpr(sb, returnStmt->value());
        }
        sb.append(L";");
    }
    else if (auto staticCast = dynamic_cast<const StaticCastExpr*>(stmt))
    {
        sb.append(L"static_cast<" + staticCast->type()->name() + L">(");
        visitExpr(sb, staticCast->expr());
        sb.append(L")");
    }
    else if (auto switchStmt = dynamic_cast<const SwitchStmt*>(stmt))
    {
        sb.append(L"switch (");
        visitExpr(sb, switchStmt->cond());
        sb.append(L")\n");
        sb.indent([&](){
            for (auto case_stmt : switchStmt->cases())
            {
                visitExpr(sb, case_stmt);
            }
        });
    }
    else if (auto unary = dynamic_cast<const UnaryExpr*>(stmt))
    {
        String op_name = L"";
        switch (unary->op())
        {
        case UnaryOp::PLUS:
            op_name = L" + ";
            break;
        case UnaryOp::MINUS:
            op_name = L" - ";
            break;
        case UnaryOp::NOT:
            op_name = L" ! ";
            break;
        case UnaryOp::BIT_NOT:
            op_name = L" ~ ";
            break;
        default:
            assert(false && "Unsupported unary operation");
        }
        
        sb.append(op_name);
        visitExpr(sb, unary->expr());
    }
    else if (auto declStmt = dynamic_cast<const DeclStmt*>(stmt))
    {
        if (auto decl = dynamic_cast<const VarDecl*>(declStmt->decl()))
        {
            sb.append(decl->type().name() + L" " + decl->name());
            if (auto init = decl->initializer())
            {
                sb.append(L" = ");
                visitExpr(sb, init);
            }
            sb.append(L";");
        }
    }
    else if (auto whileStmt = dynamic_cast<const WhileStmt*>(stmt))
    {
        sb.append(L"while (");
        visitExpr(sb, whileStmt->cond());
        sb.append(L") ");
        visitExpr(sb, whileStmt->body());
    }
    else
    {
        sb.append_expr(L"UnknownExpr ");
    }

    if (auto parent = stmt->parent())
    {
        bool IsStatement = dynamic_cast<const CompoundStmt*>(parent) ||
                           dynamic_cast<const WhileStmt*>(parent) ||
                           dynamic_cast<const SwitchStmt*>(parent);
        if (IsStatement)
            sb.endline();
    }
}

void HLSLGenerator::visit(SourceBuilderNew& sb, const skr::SSL::TypeDecl* typeDecl)
{
    using namespace skr::SSL;
    if (typeDecl->is_builtin())
    {
        sb.append(L"//builtin type: ");
        sb.append(typeDecl->name());
        sb.append(L", size: " + std::to_wstring(typeDecl->size()));
        sb.append(L", align: " + std::to_wstring(typeDecl->alignment()));
        sb.endline();
    }
    else
    {
        sb.append(L"struct " + typeDecl->name());
        sb.endline(L'{');
        for (auto field : typeDecl->fields())
        {
            sb.append(L"    " + field->type().name() + L" " + field->name() + L";\n");
        }
        sb.append(L"};\n");
    }        
}

void HLSLGenerator::visit(SourceBuilderNew& sb, const skr::SSL::FunctionDecl* funcDecl)
{
    using namespace skr::SSL;
    if (auto body = funcDecl->body())
    {
        sb.append(funcDecl->return_type()->name() + L" " + funcDecl->name() + L"(");
        for (size_t i = 0; i < funcDecl->parameters().size(); i++)
        {
            auto param = funcDecl->parameters()[i];
            String content = param->type().name() + L" " + param->name();
            if (i > 0)
                content = L", " + content;
            sb.append(content);
        }
        sb.endline(L')');
        visitExpr(sb, funcDecl->body());
    }
    else
    {
        sb.append(L"// " + funcDecl->return_type()->name() + L" " + funcDecl->name() + L"();\n");
    }
}

String HLSLGenerator::generate_code(SourceBuilderNew& sb, const AST& ast)
{
    using namespace skr::SSL;
    for (const auto& type : ast.types())
    {
        visit(sb, type);
    }
    
    for (const auto& func : ast.funcs())
    {
        visit(sb, func);
    }

    return sb.build(SourceBuilderNew::line_builder_code);
}

} // namespace skr::SSL