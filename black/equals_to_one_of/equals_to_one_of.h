#pragma once

template<typename T, typename... Args>
bool
EqualsToOneOf(const T& val, const Args&... args)
{
  return ((val == args) || ...);
}

