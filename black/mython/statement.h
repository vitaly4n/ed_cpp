#pragma once

#include "object.h"
#include "object_holder.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class TestRunner;

namespace Ast {

struct Statement
{
  virtual ~Statement() = default;
  virtual ObjectHolder Execute(Runtime::Closure& closure) = 0;
};

template<typename T>
struct ValueStatement : Statement
{
  T value_;

  explicit ValueStatement(T v)
    : value_(std::move(v))
  {}

  ObjectHolder Execute(Runtime::Closure&) override { return ObjectHolder::Share(value_); }
};

using NumericConst = ValueStatement<Runtime::Number>;
using StringConst = ValueStatement<Runtime::String>;
using BoolConst = ValueStatement<Runtime::Bool>;

struct VariableValue : Statement
{
  std::vector<std::string> dotted_ids_;

  explicit VariableValue(std::string var_name);
  explicit VariableValue(std::vector<std::string> dotted_ids);
  ObjectHolder Execute(Runtime::Closure& closure) override;
};

struct Assignment : Statement
{
  std::string var_name_;
  std::unique_ptr<Statement> right_value_;

  Assignment(std::string var, std::unique_ptr<Statement> rv);
  ObjectHolder Execute(Runtime::Closure& closure) override;
};

struct FieldAssignment : Statement
{
  VariableValue object_;
  std::string field_name_;
  std::unique_ptr<Statement> right_value_;

  FieldAssignment(VariableValue object, std::string field_name, std::unique_ptr<Statement> rv);
  ObjectHolder Execute(Runtime::Closure& closure) override;
};

struct None : Statement
{
  ObjectHolder Execute(Runtime::Closure&) override { return ObjectHolder(); }
};

class Print : public Statement
{
public:
  explicit Print(std::unique_ptr<Statement> argument);
  explicit Print(std::vector<std::unique_ptr<Statement>> args);

  static std::unique_ptr<Print> Variable(std::string name);

  ObjectHolder Execute(Runtime::Closure& closure) override;

  static void SetOutputStream(std::ostream& output_stream);

private:
  std::vector<std::unique_ptr<Statement>> args_;
  static std::ostream* output_;
};

struct MethodCall : Statement
{
  std::unique_ptr<Statement> object_;
  std::string method_;
  std::vector<std::unique_ptr<Statement>> args_;

  MethodCall(std::unique_ptr<Statement> object, std::string method, std::vector<std::unique_ptr<Statement>> args);

  ObjectHolder Execute(Runtime::Closure& closure) override;
};

struct NewInstance : Statement
{
  const Runtime::Class& class_;
  std::vector<std::unique_ptr<Statement>> args_;

  NewInstance(const Runtime::Class& cls);
  NewInstance(const Runtime::Class& cls, std::vector<std::unique_ptr<Statement>> args);
  ObjectHolder Execute(Runtime::Closure& closure) override;
};

class UnaryOperation : public Statement
{
public:
  UnaryOperation(std::unique_ptr<Statement> argument)
    : argument_(std::move(argument))
  {}

protected:
  std::unique_ptr<Statement> argument_;
};

class Stringify : public UnaryOperation
{
public:
  using UnaryOperation::UnaryOperation;
  ObjectHolder Execute(Runtime::Closure& closure) override;
};

class BinaryOperation : public Statement
{
public:
  BinaryOperation(std::unique_ptr<Statement> lhs, std::unique_ptr<Statement> rhs)
    : lhs_(std::move(lhs))
    , rhs_(std::move(rhs))
  {}

protected:
  std::unique_ptr<Statement> lhs_, rhs_;
};

class Add : public BinaryOperation
{
public:
  using BinaryOperation::BinaryOperation;
  ObjectHolder Execute(Runtime::Closure& closure) override;
};

class Sub : public BinaryOperation
{
public:
  using BinaryOperation::BinaryOperation;
  ObjectHolder Execute(Runtime::Closure& closure) override;
};

class Mult : public BinaryOperation
{
public:
  using BinaryOperation::BinaryOperation;
  ObjectHolder Execute(Runtime::Closure& closure) override;
};

class Div : public BinaryOperation
{
public:
  using BinaryOperation::BinaryOperation;
  ObjectHolder Execute(Runtime::Closure& closure) override;
};

class Or : public BinaryOperation
{
public:
  using BinaryOperation::BinaryOperation;
  ObjectHolder Execute(Runtime::Closure& closure) override;
};

class And : public BinaryOperation
{
public:
  using BinaryOperation::BinaryOperation;
  ObjectHolder Execute(Runtime::Closure& closure) override;
};

class Not : public UnaryOperation
{
public:
  using UnaryOperation::UnaryOperation;
  ObjectHolder Execute(Runtime::Closure& closure) override;
};

class Compound : public Statement
{
public:
  template<typename... Args>
  explicit Compound(Args&&... args)
  {
    (statements_.push_back(std::forward<Args>(args)), ...);
  }

  void AddStatement(std::unique_ptr<Statement> stmt) { statements_.push_back(std::move(stmt)); }

  ObjectHolder Execute(Runtime::Closure& closure) override;

private:
  std::vector<std::unique_ptr<Statement>> statements_;
};

class Return : public Statement
{
public:
  explicit Return(std::unique_ptr<Statement> statement)
    : statement_(std::move(statement))
  {}

  ObjectHolder Execute(Runtime::Closure& closure) override;

private:
  std::unique_ptr<Statement> statement_;
};

class ClassDefinition : public Statement
{
public:
  explicit ClassDefinition(ObjectHolder cls);

  ObjectHolder Execute(Runtime::Closure& closure) override;

private:
  ObjectHolder cls_;
  const std::string& class_name_;
};

class IfElse : public Statement
{
public:
  IfElse(std::unique_ptr<Statement> condition,
         std::unique_ptr<Statement> if_body,
         std::unique_ptr<Statement> else_body);

  ObjectHolder Execute(Runtime::Closure& closure) override;

private:
  std::unique_ptr<Statement> condition_, if_body_, else_body_;
};

class Comparison : public Statement
{
public:
  using Comparator = std::function<bool(const ObjectHolder&, const ObjectHolder&)>;

  Comparison(Comparator cmp, std::unique_ptr<Statement> lhs, std::unique_ptr<Statement> rhs);

  ObjectHolder Execute(Runtime::Closure& closure) override;

private:
  Comparator comparator_;
  std::unique_ptr<Statement> left_, right_;
};

void
RunUnitTests(TestRunner& tr);

}

using Statement = Ast::Statement;
