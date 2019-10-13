#pragma once

#include "object_holder.h"

#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace Ast {
struct Statement;
}

class TestRunner;

namespace Runtime {

class Object
{
public:
  virtual ~Object() = default;
  virtual void Print(std::ostream& os) = 0;
};

template<typename T>
class ValueObject : public Object
{
public:
  ValueObject(T v)
    : value_(v)
  {}

  void Print(std::ostream& os) override { os << value_; }

  const T& GetValue() const { return value_; }

private:
  T value_;
};

using String = ValueObject<std::string>;
using Number = ValueObject<int>;

class Bool : public ValueObject<bool>
{
public:
  using ValueObject<bool>::ValueObject;
  void Print(std::ostream& os) override;
};

struct Method
{
  std::string name;
  std::vector<std::string> formal_params;
  std::unique_ptr<Ast::Statement> body;
};

class Class : public Object
{
public:
  explicit Class(std::string name, std::vector<Method> methods, const Class* parent);
  const Method* GetMethod(const std::string& name) const;
  const std::string& GetName() const;
  void Print(std::ostream& os) override;

private:
  std::string name_;
  std::vector<Method> methods_;
  const Class* parent_ = nullptr;
  std::unordered_map<std::string, const Method*> vtable_;
};

class ClassInstance : public Object
{
public:
  explicit ClassInstance(const Class& cls);

  void Print(std::ostream& os) override;

  ObjectHolder Call(const std::string& method, const std::vector<ObjectHolder>& actual_args);
  bool HasMethod(const std::string& method, size_t argument_count) const;

  Closure& Fields();
  const Closure& Fields() const;

private:
  const Class& class_;
  Closure closure_;
};

void
RunObjectsTests(TestRunner& test_runner);

}