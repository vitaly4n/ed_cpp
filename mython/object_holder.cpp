#include "object_holder.h"
#include "object.h"

namespace Runtime {

ObjectHolder
ObjectHolder::Share(Object& object)
{
  return ObjectHolder(std::shared_ptr<Object>(&object, [](auto*) { /* do nothing */ }));
}

ObjectHolder
ObjectHolder::None()
{
  return ObjectHolder();
}

Object& ObjectHolder::operator*()
{
  return *Get();
}

const Object& ObjectHolder::operator*() const
{
  return *Get();
}

Object* ObjectHolder::operator->()
{
  return Get();
}

const Object* ObjectHolder::operator->() const
{
  return Get();
}

Object*
ObjectHolder::Get()
{
  return data_.get();
}

const Object*
ObjectHolder::Get() const
{
  return data_.get();
}

ObjectHolder::operator bool() const
{
  return Get();
}

bool
IsTrue(ObjectHolder object)
{
  if (auto condition_bool = object.TryAs<Bool>()) {
    return condition_bool->GetValue();
  } else if (auto condition_num = object.TryAs<Number>()) {
    return condition_num->GetValue() != 0;
  } else if (auto condition_str = object.TryAs<String>()) {
    return !condition_str->GetValue().empty();
  } else if (auto condition_obj = object.TryAs<ClassInstance>()) {
    return true;
  }
  return false;
}

}
