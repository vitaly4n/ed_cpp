#pragma once

#include <iterator>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template<typename It>
class Range
{
public:
  using ValueType = typename std::iterator_traits<It>::value_type;

  Range(It begin, It end)
    : begin_(begin)
    , end_(end)
  {}
  It begin() const { return begin_; }
  It end() const { return end_; }

private:
  It begin_;
  It end_;
};

template<typename C>
auto
AsRange(const C& container)
{
  return Range{ std::begin(container), std::end(container) };
}

template<typename It>
size_t
ComputeUniqueItemsCount(Range<It> range)
{
  return std::unordered_set<typename Range<It>::ValueType>{ range.begin(), range.end() }.size();
}

template<typename K, typename V>
const V*
GetValuePointer(const std::unordered_map<K, V>& map, const K& key)
{
  if (auto it = map.find(key); it != end(map)) {
    return &it->second;
  } else {
    return nullptr;
  }
}

std::string_view
Strip(std::string_view line);

template<typename It>
class Paginator
{
public:
  using Page = Range<It>;
  using Pages = std::vector<Page>;
  using iterator = typename Pages::iterator;
  using const_iterator = typename Pages::const_iterator;

  Paginator(It first, It last, size_t page_size)
    : first_(first)
    , last_(last)
    , page_size_(page_size)
  {
    pages_.reserve(std::distance(first, last) / page_size + 1);
    while (first != last) {
      if (std::distance(first, last) >= page_size) {
        auto next = std::next(first, page_size);
        pages_.emplace_back(first, next);
        first = next;
      } else {
        pages_.emplace_back(first, last);
        first = last;
      }
    }
  }

  const_iterator begin() const { return std::begin(pages_); }
  const_iterator end() const { return std::end(pages_); }

private:
  It first_;
  It last_;
  size_t page_size_;

  Pages pages_;
};

template<typename It>
auto Paginate(It first, It last, size_t page_size) {
  return Paginator(first, last, page_size);
}

inline bool IsZero(double val) {
  return std::abs(val) < 1e-10;
}
