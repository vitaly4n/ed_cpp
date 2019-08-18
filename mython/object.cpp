#include "object.h"
#include "statement.h"

#include <sstream>
#include <string_view>

using namespace std;

namespace Runtime {

void
ClassInstance::Print(std::ostream& os)
{
  // TODO:
}

bool
ClassInstance::HasMethod(const std::string& method, size_t argument_count) const
{
  // TODO:
  return false;
}

const Closure&
ClassInstance::Fields() const
{
  // TODO:
  static Closure c;
  return c;
}

Closure&
ClassInstance::Fields()
{
  // TODO:
  static Closure c;
  return c;
}

ClassInstance::ClassInstance(const Class& cls) {}

ObjectHolder
ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args)
{
  // TODO:
  static ObjectHolder h;
  return h;
}

Class::Class(std::string name, std::vector<Method> methods, const Class* parent) {}

const Method*
Class::GetMethod(const std::string& name) const
{
  // TODO:
  return nullptr;
}

void
Class::Print(ostream& os)
{
  // TODO:
}

const std::string&
Class::GetName() const
{
  // TODO:
  static std::string str;
  return str;
}

void
Bool::Print(std::ostream& os)
{
  os << (GetValue() ? "True" : "False");
}

} /* namespace Runtime */
