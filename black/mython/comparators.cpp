#include "comparators.h"
#include "object.h"
#include "object_holder.h"

#include <functional>
#include <optional>
#include <sstream>

using namespace std;

namespace Runtime {

template<typename T, typename Op>
optional<bool>
ExecuteOp(const ObjectHolder& lhs, const ObjectHolder& rhs, Op op)
{
  auto lhs_val = lhs.TryAs<ValueObject<T>>();
  auto rhs_val = rhs.TryAs<ValueObject<T>>();
  if (!lhs_val || !rhs_val) {
    return nullopt;
  }
  return op(lhs_val->GetValue(), rhs_val->GetValue());
}

template<typename Op>
bool
ExecuteOpVar(const ObjectHolder& lhs, const ObjectHolder& rhs, Op op)
{
  if (auto opt = ExecuteOp<int>(lhs, rhs, op)) {
    return *opt;
  } else if (auto opt = ExecuteOp<string>(lhs, rhs, op)) {
    return *opt;
  } else if (auto opt = ExecuteOp<bool>(lhs, rhs, op)) {
    return *opt;
  }
  return false;
}

bool
Equal(ObjectHolder lhs, ObjectHolder rhs)
{
  return ExecuteOpVar(lhs, rhs, [](const auto& left, const auto& right) { return left == right; });
}

bool
Less(ObjectHolder lhs, ObjectHolder rhs)
{
  return ExecuteOpVar(lhs, rhs, [](const auto& left, const auto& right) { return left < right; });
}

} /* namespace Runtime */
