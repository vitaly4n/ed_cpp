#include "object.h"
#include "statement.h"

#include <sstream>
#include <stack>
#include <string_view>

using namespace std;

namespace Runtime {

void
ClassInstance::Print(std::ostream& os)
{
  static std::string str_method_name = "__str__";
  if (HasMethod(str_method_name, 0)) {
    const ObjectHolder str_holder = Call(str_method_name, {});
    os << str_holder.TryAs<String>();
  } else {
    os << this;
  }
}

bool
ClassInstance::HasMethod(const std::string& method, size_t argument_count) const
{
  if (auto method_func = class_.GetMethod(method)) {
    return method_func->formal_params.size() == argument_count;
  }
  return false;
}

const Closure&
ClassInstance::Fields() const
{
  return closure_;
}

Closure&
ClassInstance::Fields()
{
  return closure_;
}

ClassInstance::ClassInstance(const Class& cls)
  : class_(cls)
{}

ObjectHolder
ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args)
{
  auto method_func = class_.GetMethod(method);
  if (!method_func || method_func->formal_params.size() != actual_args.size()) {
    return ObjectHolder();
  }

  Closure method_closure;
  method_closure["self"] = ObjectHolder::Share(*this);
  for (unsigned i = 0; i < actual_args.size(); ++i) {
    method_closure[method_func->formal_params[i]] = actual_args[i];
  }
  return method_func->body->Execute(method_closure);
}

Class::Class(std::string name, std::vector<Method> methods, const Class* parent)
  : name_(std::move(name))
  , methods_(std::move(methods))
  , parent_(parent)
{
  std::stack<const Class*> ancestor_stack;
  for (const Class* cls = this; cls; cls = cls->parent_) {
    ancestor_stack.push(cls);
  }

  while (!ancestor_stack.empty()) {
    const Class* ancestor = ancestor_stack.top();
    ancestor_stack.pop();
    for (const auto& method : ancestor->methods_) {
      vtable_[method.name] = &method;
    }
  }
}

const Method*
Class::GetMethod(const std::string& name) const
{
  auto it = vtable_.find(name);
  return it != std::end(vtable_) ? it->second : nullptr;
}

void
Class::Print(ostream& os)
{
  os << name_;
}

const std::string&
Class::GetName() const
{
  return name_;
}

void
Bool::Print(std::ostream& os)
{
  os << (GetValue() ? "True" : "False");
}

} /* namespace Runtime */
