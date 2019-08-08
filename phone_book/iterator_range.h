#pragma once

#include <algorithm>

template<typename It>
class IteratorRange
{
public:
  IteratorRange(It first, It last)
    : first_(first)
    , last_(last)
  {}

  It begin() const { return first_; }

  It end() const { return last_; }

  size_t size() const { return last_ - first_; }

private:
  It first_, last_;
};

template<typename Container>
auto
Head(Container& c, int top)
{
  return IteratorRange(std::begin(c), std::begin(c) + std::min<size_t>(std::max(top, 0), c.size()));
}
