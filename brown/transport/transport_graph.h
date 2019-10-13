#pragma once

#include "graph.h"
#include "router.h"

#include <memory>
#include <string>
#include <string_view>

template<typename Key, typename Val>
struct Table : std::unordered_map<Key, std::unordered_map<Key, Val>>
{
public:
  void Insert(const Key& row, const Key& column, const Val& val, bool doEmplace)
  {
    auto emplace_res = (*this)[row].emplace(column, val);
    if (!emplace_res.second && doEmplace) {
      emplace_res.first->second = val;
    }
  }

  const Val& Get(const Key& row, const Key& column) const
  {
    return at(row).at(column);
  }
};