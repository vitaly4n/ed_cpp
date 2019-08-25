#pragma once

#include <utility>


template<typename F, typename... Args>
void ApplyToMany(F func, Args&&... args)
{
  (func(std::forward<Args>(args)), ...);
}
