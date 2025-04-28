#include "ExpressionTranslator.hpp"

namespace skr::SSL
{

bool ExprTranslator::VisitStmt(clang::Stmt* x)
{
    /*
    if (IF(x))
        b = AddExpr(IF->Then()); c = AddExpr(IF->Else()); Expression.IfThenElse(a, b, c);    
    */
    if (auto cxxDecl = clang::dyn_cast<clang::DeclStmt>(x))
    {
        llvm::outs() << x->getStmtClassName() << "\n";
        //for (auto child : cxxDecl->children())
        //    child->dump();

        const auto declGroup = cxxDecl->getDeclGroup();
        for (auto decl : declGroup)
        {
            if (auto var = clang::dyn_cast<clang::VarDecl>(decl))
            {
                /*
                llvm::outs() << var->getName() << " " << var->getDeclName() << "\n";
                clang::QualType t = var->getType();
                if (auto R = t.getCanonicalType()->getAsRecordDecl())
                {
                    R->dump();
                }
                llvm::outs() << "VarDecl: " << var->getName() << "\n";
                */
                return true;
            }
        }
    }

    return true;
}

bool ExprTranslator::VisitRecordDecl(clang::RecordDecl* x)
{
    const bool isNormalClass = x->isCompleteDefinition()&& !x->isTemplated();
    const bool isTemplateSpec = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(x) && x->isCompleteDefinition();
    if (isNormalClass || isTemplateSpec)
    {
        // x->dump();
    }
    return true;
}

bool ExprTranslator::VisitFunctionDecl(clang::FunctionDecl* x)
{
    return true;
}

bool ExprTranslator::VisitParmVarDecl(clang::ParmVarDecl* x)
{
    x->dump();
    x->getBody()->dump();
    return true;
}

}