#pragma once

#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#define STRINGIFY(m) #m
#define STRINGIFY_2(m) STRINGIFY(m)

template<typename It>
class Range
{
public:
  Range(It begin, It end)
    : begin_(begin)
    , end_(end)
  {}
  It begin() const { return begin_; }
  It end() const { return end_; }

  bool empty() { return begin_ == end_; }

private:
  It begin_;
  It end_;
};

template<typename It>
auto
MakeRange(It first, It last)
{
  return Range<It>(first, last);
}

inline std::pair<std::string_view, std::optional<std::string_view>>
SplitTwoStrict(std::string_view s, std::string_view delimiter = " ")
{
  const std::size_t pos = s.find(delimiter);
  if (pos == s.npos) {
    return { s, std::nullopt };
  } else {
    return { s.substr(0, pos), s.substr(pos + delimiter.length()) };
  }
}

inline std::vector<std::string_view>
Split(std::string_view s, std::string_view delimiter = " ")
{
  using std::string_view;
  using std::vector;

  vector<string_view> parts;
  if (s.empty()) {
    return parts;
  }
  while (true) {
    const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
    parts.push_back(lhs);
    if (!rhs_opt) {
      break;
    }
    s = *rhs_opt;
  }
  return parts;
}

inline std::pair<std::string_view, std::string_view>
SplitTwo(std::string_view s, std::string_view delimiter = " ")
{
  const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
  return { lhs, rhs_opt.value_or("") };
}

inline std::string_view
ReadToken(std::string_view& s, std::string_view delimiter = " ")
{
  const auto [lhs, rhs] = SplitTwo(s, delimiter);
  s = rhs;
  return lhs;
}

template<typename T>
inline T
ConvertFromView(std::string_view str)
{
  // copying...

  T result{};
  std::istringstream is(str.data());
  is >> result;
  return result;
}