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
{
  dotted_ids_.push_back(std::move(var_name));
}

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
  return std::make_unique<Print>(std::make_unique<VariableValue>(std::move(var)));
}

Print::Print(unique_ptr<Statement> argument)
{
  args_.push_back(std::move(argument));
}

Print::Print(vector<unique_ptr<Statement>> args)
  : args_(std::move(args))
{}

ObjectHolder
Print::Execute(Closure& closure)
{
  ObjectHolder string_holder;
  for (unsigned i = 0; i < args_.size(); ++i) {
    if (i != 0) {
      (*output_) << ' ';
    }
    if (auto ex_res = args_[i]->Execute(closure)) {
      ex_res->Print(*output_);
    } else {
      (*output_) << "None";
    }
  }
  (*output_) << '\n';
  return ObjectHolder();
}

ostream* Print::output_ = &cout;

void
Print::SetOutputStream(ostream& output_stream)
{
  output_ = &output_stream;
}

MethodCall::MethodCall(std::unique_ptr<Statement> object,
                       std::string method,
                       std::vector<std::unique_ptr<Statement>> args)
  : object_(std::move(object))
  , method_(std::move(method))
  , args_(std::move(args))
{}

ObjectHolder
MethodCall::Execute(Closure& closure)
{
  auto inst_var = object_->Execute(closure);
  auto inst = inst_var.TryAs<Runtime::ClassInstance>();
  if (!inst) {
    throw std::runtime_error("cannot call method on non-object instance");
  }
  if (!inst->HasMethod(method_, args_.size())) {
    throw std::runtime_error("invalid method name");
  }

  std::vector<ObjectHolder> method_args;
  method_args.reserve(args_.size());
  for (const auto& arg : args_) {
    method_args.push_back(arg->Execute(closure));
  }
  return inst->Call(method_, method_args);
}

ObjectHolder
Stringify::Execute(Closure& closure)
{
  std::ostringstream output;
  argument_->Execute(closure)->Print(output);
  return ObjectHolder::Own(Runtime::String(output.str()));
}

template<typename... Args>
struct ArithmeticOp
{};

template<typename T, typename... Args>
struct ArithmeticOp<T, Args...>
{
  template<typename Op>
  static ObjectHolder _(const ObjectHolder& lhs, const ObjectHolder& rhs, Op op)
  {
    using namespace Runtime;
    auto lhs_val = lhs.TryAs<ValueObject<T>>();
    auto rhs_val = rhs.TryAs<ValueObject<T>>();
    if (lhs_val && rhs_val) {
      auto res = op(lhs_val->GetValue(), rhs_val->GetValue());
      return ObjectHolder::Own(ValueObject<T>(res));
    }
    return ArithmeticOp<Args...>::_(lhs, rhs, op);
  }
};

template<>
struct ArithmeticOp<>
{
  template<typename Op>
  static ObjectHolder _(const ObjectHolder&, const ObjectHolder&, Op)
  {
    throw std::runtime_error("invalid binary operation");
  }
};

template<typename T>
struct ArithmeticOp<T>
{
  template<typename Op>
  static ObjectHolder _(const ObjectHolder& lhs, const ObjectHolder& rhs, Op op)
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
};

ObjectHolder
Add::Execute(Closure& closure)
{
  ObjectHolder lhs_val = lhs_->Execute(closure);
  ObjectHolder rhs_val = rhs_->Execute(closure);

  if (auto lhs_inst = lhs_val.TryAs<Runtime::ClassInstance>()) {
    static const std::string add_method_name = "__add__";
    return lhs_inst->Call(add_method_name, {rhs_val});
  }

  return ArithmeticOp<int, std::string>::_(lhs_val, rhs_val, [](const auto& lhs, const auto& rhs) {
    return lhs + rhs;
  });
}

ObjectHolder
Sub::Execute(Closure& closure)
{
  ObjectHolder lhs_val = lhs_->Execute(closure);
  ObjectHolder rhs_val = rhs_->Execute(closure);

  return ArithmeticOp<int>::_(lhs_val, rhs_val, [](auto lhs, auto rhs) { return lhs - rhs; });
}

ObjectHolder
Mult::Execute(Runtime::Closure& closure)
{
  ObjectHolder lhs_val = lhs_->Execute(closure);
  ObjectHolder rhs_val = rhs_->Execute(closure);

  return ArithmeticOp<int>::_(lhs_val, rhs_val, [](auto lhs, auto rhs) { return lhs * rhs; });
}

ObjectHolder
Div::Execute(Runtime::Closure& closure)
{
  ObjectHolder lhs_val = lhs_->Execute(closure);
  ObjectHolder rhs_val = rhs_->Execute(closure);

  return ArithmeticOp<int>::_(lhs_val, rhs_val, [](auto lhs, auto rhs) { return lhs / rhs; });
}

ObjectHolder
Compound::Execute(Closure& closure)
{
  for (const auto& statement : statements_) {
    ObjectHolder res = statement->Execute(closure);
    auto returnIt = closure.find("__return__");
    if (returnIt != std::end(closure)) {
      return returnIt->second;
    }
  }

  return ObjectHolder();
}

ObjectHolder
Return::Execute(Closure& closure)
{
  ObjectHolder res = statement_->Execute(closure);
  closure["__return__"] = res;
  return res;
}

ClassDefinition::ClassDefinition(ObjectHolder cls)
  : cls_(std::move(cls))
  , class_name_(cls_.TryAs<Runtime::Class>()->GetName())
{}

ObjectHolder
ClassDefinition::Execute(Runtime::Closure& closure)
{
  return cls_;
}

FieldAssignment::FieldAssignment(VariableValue object, std::string field_name, std::unique_ptr<Statement> rv)
  : object_(std::move(object))
  , field_name_(std::move(field_name))
  , right_value_(std::move(rv))
{}

ObjectHolder
FieldAssignment::Execute(Runtime::Closure& closure)
{
  Closure& object_closure = object_.Execute(closure).TryAs<Runtime::ClassInstance>()->Fields();
  ObjectHolder& field = object_closure[field_name_];
  field = right_value_->Execute(closure);
  return field;
}

IfElse::IfElse(std::unique_ptr<Statement> condition,
               std::unique_ptr<Statement> if_body,
               std::unique_ptr<Statement> else_body)
  : condition_(std::move(condition))
  , if_body_(std::move(if_body))
  , else_body_(std::move(else_body))
{}

ObjectHolder
IfElse::Execute(Runtime::Closure& closure)
{
  using namespace Runtime;

  ObjectHolder condition_val = condition_->Execute(closure);
  if (IsTrue(condition_val)) {
    return if_body_->Execute(closure);
  } else if (else_body_) {
    return else_body_->Execute(closure);
  }
  return ObjectHolder();
}

ObjectHolder
Or::Execute(Runtime::Closure& closure)
{
  const bool res = IsTrue(lhs_->Execute(closure)) || IsTrue(rhs_->Execute(closure));
  return ObjectHolder::Own(Runtime::Bool(res));
}

ObjectHolder
And::Execute(Runtime::Closure& closure)
{
  const bool res = IsTrue(lhs_->Execute(closure)) && IsTrue(rhs_->Execute(closure));
  return ObjectHolder::Own(Runtime::Bool(res));
}

ObjectHolder
Not::Execute(Runtime::Closure& closure)
{
  const bool res = !IsTrue(argument_->Execute(closure));
  return ObjectHolder::Own(Runtime::Bool(res));
}

Comparison::Comparison(Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs)
  : comparator_(std::move(cmp))
  , left_(std::move(lhs))
  , right_(std::move(rhs))
{}

ObjectHolder
Comparison::Execute(Runtime::Closure& closure)
{
  const bool res = comparator_(left_->Execute(closure), right_->Execute(closure));
  return ObjectHolder::Own(Runtime::Bool(res));
}

NewInstance::NewInstance(const Runtime::Class& cls, std::vector<std::unique_ptr<Statement>> args)
  : class_(cls)
  , args_(std::move(args))
{}

NewInstance::NewInstance(const Runtime::Class& cls)
  : NewInstance(cls, {})
{}

ObjectHolder
NewInstance::Execute(Runtime::Closure& closure)
{
  static const std::string init_method_name = "__init__";

  Runtime::ClassInstance inst(class_);
  if (inst.HasMethod(init_method_name, args_.size())) {
    std::vector<ObjectHolder> init_args;
    init_args.reserve(args_.size());
    for (const auto& arg : args_) {
      init_args.push_back(arg->Execute(closure));
    }
    inst.Call(init_method_name, init_args);
  } else if (!args_.empty()) {
    throw std::runtime_error("invalid number of _init_ parameters");
  }
  return ObjectHolder::Own(std::move(inst));
}

} /* namespace Ast */
