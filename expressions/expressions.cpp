#include "Common.h"

#include "assert.h"
#include <memory>
#include <sstream>
#include <string>
#include <utility>

class BinaryExpression : public Expression
{
public:
  BinaryExpression(ExpressionPtr&& left, ExpressionPtr&& right)
    : left_{ std::move(left) }
    , right_{ std::move(right) }
  {}

  int Evaluate() const override final
  {
    assert(left_);
    assert(right_);
    return OperationEval(left_->Evaluate(), right_->Evaluate());
  }

  std::string ToString() const override final
  {
    assert(left_);
    assert(right_);
    std::ostringstream os;
    os << "(" << left_->ToString() << ")"<< OperationSign() << "(" << right_->ToString()
       << ")";
    return os.str();
  }

  virtual int OperationEval(int left, int right) const = 0;
  virtual std::string OperationSign() const = 0;

private:
  ExpressionPtr left_;
  ExpressionPtr right_;
};

class SumExpression : public BinaryExpression
{
public:
  using BinaryExpression::BinaryExpression;

  int OperationEval(int left, int right) const override { return left + right; }
  std::string OperationSign() const override { return "+"; }
};

class ProductExpression : public BinaryExpression
{
public:
  using BinaryExpression::BinaryExpression;

  int OperationEval(int left, int right) const override { return left * right; }
  std::string OperationSign() const override { return "*"; }
};

class ValueExpression : public Expression
{
public:
  ValueExpression(int value)
    : value_{ value }
  {}

  int Evaluate() const override { return value_; }
  std::string ToString() const override
  {
    return std::to_string(value_);
  }

private:
  int value_;
};

ExpressionPtr
Value(int value)
{
  return std::make_unique<ValueExpression>(value);
}

ExpressionPtr
Sum(ExpressionPtr left, ExpressionPtr right)
{
  return std::make_unique<SumExpression>(std::move(left), std::move(right));
}

ExpressionPtr
Product(ExpressionPtr left, ExpressionPtr right)
{
  return std::make_unique<ProductExpression>(std::move(left), std::move(right));
}
