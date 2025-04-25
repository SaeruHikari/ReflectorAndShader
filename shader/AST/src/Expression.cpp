#include "SSL/Expression.hpp"

namespace skr::SSL {

BinaryExpression* ExpressionFactory::Add(Expression* left, Expression* right)
{
    auto exp = new BinaryExpression(left, right, BinaryOp::ADD);
    _expressions.emplace_back(exp);
    return exp;
}

BinaryExpression* ExpressionFactory::Assign(Expression* left, Expression* right)
{
    auto exp = new BinaryExpression(left, right, BinaryOp::ASSIGN);
    _expressions.emplace_back(exp);
    return exp;
}

BlockExpression* ExpressionFactory::Block(const std::vector<ParameterExpression*>& variables, const std::vector<Expression*>& expressions)
{
    auto exp = new BlockExpression(variables, expressions);
    _expressions.emplace_back(exp);
    return exp;
}

BlockExpression* ExpressionFactory::Block()
{
    auto exp = new BlockExpression({}, {});
    _expressions.emplace_back(exp);
    return exp;
}

ConstantExpression* ExpressionFactory::Constant(const FloatSemantics& v) 
{ 
    auto exp = new ConstantExpression(v); 
    _expressions.emplace_back(exp);
    return exp;
}

ParameterExpression* ExpressionFactory::Variable(const Type& type) 
{  
    auto exp = new ParameterExpression(type); 
    _expressions.emplace_back(exp);
    return exp;
}

ParameterExpression* ExpressionFactory::Variable(const Type& type, const Name& name) 
{  
    auto exp = new ParameterExpression(type, name); 
    _expressions.emplace_back(exp);
    return exp;
}

BlockExpression* BlockExpression::add_variable(ParameterExpression* variable) 
{ 
    _variables.emplace_back(variable); 
    return this; 
}

BlockExpression* BlockExpression::add_expression(Expression* expression) 
{ 
    _expressions.emplace_back(expression); 
    return this; 
}

ExpressionFactory::~ExpressionFactory()
{
    for (auto expr : _expressions)
    {
        delete expr;
    }
    _expressions.clear();
}

}