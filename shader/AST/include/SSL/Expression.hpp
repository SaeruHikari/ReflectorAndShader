#pragma once
#include <variant>
#include "Enums.hpp"
#include "Type.hpp"

namespace skr::SSL {

struct BinaryExpression;
struct ParameterExpression;
struct BlockExpression;
struct ConstantExpression;
using FloatSemantics = String;

struct Expression
{
    virtual ~Expression() = default;
};

struct BinaryExpression : Expression
{
public:
    const Expression* left() const { return _left; }
    const Expression* right() const { return _right; }
    const BinaryOp op() const { return _op; }

private:
    friend struct ExpressionFactory;
    BinaryExpression(Expression* left, Expression* right, BinaryOp op) : _left(left), _right(right), _op(op) {}
    Expression* _left = nullptr;
    Expression* _right = nullptr;
    const BinaryOp _op = BinaryOp::ADD;
};

struct ParameterExpression : Expression
{
public:
    const Type& type() const { return _type; }
    const Name& name() const { return _name; }
    
private:
    friend struct ExpressionFactory;
    ParameterExpression(const Type& type) : _type(type) {}
    ParameterExpression(const Type& type, const Name& name) : _type(type), _name(name) {}
    const Type& _type;
    const Name _name = u8"__INVALID_PARAMETER__";
};

struct BlockExpression : Expression
{
public:
    std::span<ParameterExpression* const> variables() const { return _variables; }
    std::span<Expression* const> expressions() const { return _expressions; }

    BlockExpression* add_variable(ParameterExpression* variable);
    BlockExpression* add_expression(Expression* expression);

    template <typename... Args>
    BlockExpression* add_expressions(Args&&... args)
    {
        (_expressions.emplace_back(args), ...);
        return this;
    }

    template <typename... Args>
    BlockExpression* add_variables(Args&&... args)
    {
        (_variables.emplace_back(args), ...);
        return this;
    }

private:
    friend struct ExpressionFactory;
    BlockExpression(std::span<ParameterExpression* const> variables, std::span<Expression* const> expressions) 
        : _variables(variables.begin(), variables.end()), _expressions(expressions.begin(), expressions.end()) 
    {

    }
    std::vector<ParameterExpression*> _variables;
    std::vector<Expression*> _expressions;
};

struct ConstantExpression : Expression
{
public:
    const String v;

private:
    friend struct ExpressionFactory;
    inline ConstantExpression(const String& v) : v(v) {}
};

struct ExpressionFactory
{
public:
    ExpressionFactory() = default;
    ~ExpressionFactory();

    BinaryExpression* Add(Expression* left, Expression* right);
    BinaryExpression* Assign(Expression* left, Expression* right);

    BlockExpression* Block();
    BlockExpression* Block(const std::vector<ParameterExpression*>& variables, const std::vector<Expression*>& expressions);

    ConstantExpression* Constant(const FloatSemantics& v);

    ParameterExpression* Variable(const Type& type);
    ParameterExpression* Variable(const Type& type, const Name& name);

private:
    std::vector<Expression*> _expressions;
};

}