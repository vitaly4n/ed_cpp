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
  ObjectHolder& var = closure[var_name_];
  var = right_value_->Execute(closure);
  return var;
}

Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv)
  : var_name_(std::move(var))
  , right_value_(std::move(rv))
{}

VariableValue::VariableValue(std::string var_name)
  : dotted_ids_{ var_name }
{}

VariableValue::VariableValue(std::vector<std::string> dotted_ids)
  : dotted_ids_(std::move(dotted_ids))
{}

ObjectHolder
VariableValue::Execute(Closure& closure)
{
  Closure* obj_closure = &closure;
  for (unsigned i = 0; i < dotted_ids_.size() - 1; ++i) {
    const std::string& id = dotted_ids_[i];
    ObjectHolder& obj = (*obj_closure)[id];
    if (auto class_instance_obj = obj.TryAs<Runtime::ClassInstance>()) {
      obj_closure = &class_instance_obj->Fields();
    } else {
      throw std::runtime_error("invalid field at " + id);
    }
  }
  return (*obj_closure)[dotted_ids_.back()];
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

template<typename T, typename Op>
ObjectHolder
ExecuteArithmeticOp(const ObjectHolder& lhs, const ObjectHolder& rhs, Op op)
{
  using namespace Runtime;
  auto lhs_val = lhs.TryAs<ValueObject<T>>();
  auto rhs_val = rhs.TryAs<ValueObject<T>>();
  if (!lhs_val || !rhs_val) {
    throw std::runtime_error("invalid binary operation");
  }
  auto res = op(lhs_val->GetValue(), rhs_val->GetValue());
  return ObjectHolder::Own(ValueObject<T>(res));
}

ObjectHolder
Add::Execute(Closure& closure)
{
  ObjectHolder lhs_val = lhs_->Execute(closure);
  ObjectHolder rhs_val = rhs_->Execute(closure);

  return ExecuteArithmeticOp<int>(lhs_val, rhs_val, [](auto lhs, auto rhs) { return lhs + rhs; });
}

ObjectHolder
Sub::Execute(Closure& closure)
{
  ObjectHolder lhs_val = lhs_->Execute(closure);
  ObjectHolder rhs_val = rhs_->Execute(closure);

  return ExecuteArithmeticOp<int>(lhs_val, rhs_val, [](auto lhs, auto rhs) { return lhs - rhs; });
}

ObjectHolder
Mult::Execute(Runtime::Closure& closure)
{
  ObjectHolder lhs_val = lhs_->Execute(closure);
  ObjectHolder rhs_val = rhs_->Execute(closure);

  return ExecuteArithmeticOp<int>(lhs_val, rhs_val, [](auto lhs, auto rhs) { return lhs * rhs; });
}

ObjectHolder
Div::Execute(Runtime::Closure& closure)
{
  ObjectHolder lhs_val = lhs_->Execute(closure);
  ObjectHolder rhs_val = rhs_->Execute(closure);

  return ExecuteArithmeticOp<int>(lhs_val, rhs_val, [](auto lhs, auto rhs) { return lhs / rhs; });
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
{}

ObjectHolder
FieldAssignment::Execute(Runtime::Closure& closure)
{
  Closure& object_closure = closure.at("self").TryAs<Runtime::ClassInstance>()->Fields();
  ObjectHolder& field = object_closure[field_name];
  field = right_value->Execute(closure);
  return field;
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
