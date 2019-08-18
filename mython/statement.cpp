#include "statement.h"
#include "object.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace Ast {

using Runtime::Closure;

ObjectHolder
Assignment::Execute(Closure& closure)
{
  // TODO:
  static ObjectHolder h;
  return h;
}

Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv)
{
  // TODO:
}

VariableValue::VariableValue(std::string var_name)
{
  // TODO:
}

VariableValue::VariableValue(std::vector<std::string> dotted_ids)
{
  // TODO:
}

ObjectHolder
VariableValue::Execute(Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

unique_ptr<Print>
Print::Variable(std::string var)
{
  // TODO:
  return nullptr;
}

Print::Print(unique_ptr<Statement> argument)
{
  // TODO:
}

Print::Print(vector<unique_ptr<Statement>> args)
{
  // TODO:
}

ObjectHolder
Print::Execute(Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

ostream* Print::output = &cout;

void
Print::SetOutputStream(ostream& output_stream)
{
  output = &output_stream;
}

MethodCall::MethodCall(std::unique_ptr<Statement> object,
                       std::string method,
                       std::vector<std::unique_ptr<Statement>> args)
{
  // TODO:
}

ObjectHolder
MethodCall::Execute(Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

ObjectHolder
Stringify::Execute(Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

ObjectHolder
Add::Execute(Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

ObjectHolder
Sub::Execute(Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

ObjectHolder
Mult::Execute(Runtime::Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

ObjectHolder
Div::Execute(Runtime::Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

ObjectHolder
Compound::Execute(Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

ObjectHolder
Return::Execute(Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

ClassDefinition::ClassDefinition(ObjectHolder class_)
  : class_name(class_.TryAs<Runtime::Class>()->GetName())
{
  // TODO:
}

ObjectHolder
ClassDefinition::Execute(Runtime::Closure& closure)
{
  // TODO:
}

FieldAssignment::FieldAssignment(VariableValue object, std::string field_name, std::unique_ptr<Statement> rv)
  : object(std::move(object))
  , field_name(std::move(field_name))
  , right_value(std::move(rv))
{
  // TODO:
}

ObjectHolder
FieldAssignment::Execute(Runtime::Closure& closure)
{
  // TODO:
}

IfElse::IfElse(std::unique_ptr<Statement> condition,
               std::unique_ptr<Statement> if_body,
               std::unique_ptr<Statement> else_body)
{
  // TODO:
}

ObjectHolder
IfElse::Execute(Runtime::Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

ObjectHolder
Or::Execute(Runtime::Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

ObjectHolder
And::Execute(Runtime::Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

ObjectHolder
Not::Execute(Runtime::Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

Comparison::Comparison(Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs)
{
  // TODO:
}

ObjectHolder
Comparison::Execute(Runtime::Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

NewInstance::NewInstance(const Runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args)
  : class_(class_)
  , args_(std::move(args))
{
  // TODO:
}

NewInstance::NewInstance(const Runtime::Class& class_)
  : NewInstance(class_, {})
{
  // TODO:
}

ObjectHolder
NewInstance::Execute(Runtime::Closure& closure)
{
  // TODO:
  return ObjectHolder();
}

} /* namespace Ast */
