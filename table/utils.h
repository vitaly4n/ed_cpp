#pragma once

#include <iterator>

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

template<typename Map>
class MapKeyIterator
{
public:
  MapKeyIterator(typename Map::iterator it)
    : it_(it)
  {}

  MapKeyIterator& operator++()
  {
    ++it_;
    return *this;
  }
  MapKeyIterator operator++(int) { return MapIterator(it_++); }

  template<typename T>
  bool operator==(const T& other) const
  {
    return it_ == other.it_;
  }
  template<typename T>
  bool operator!=(const T& other) const
  {
    return it_ != other.it_;
  }

  typename Map::mapped_type& operator*() { return it_->first; }
  typename Map::mapped_type* operator->() { return &it_->first; }

private:
  typename Map::iterator it_;
};

template<typename Map>
class ConstMapKeyIterator
{
public:
  ConstMapKeyIterator(typename Map::iterator it)
    : it_(it)
  {}

  ConstMapKeyIterator& operator++()
  {
    ++it_;
    return *this;
  }
  ConstMapKeyIterator operator++(int) { return MapIterator(it_++); }

  template<typename T>
  bool operator==(const T& other) const
  {
    return it_ == other.it_;
  }
  template<typename T>
  bool operator!=(const T& other) const
  {
    return it_ != other.it_;
  }
  typename Map::mapped_type const& operator*() const { return it_->first; }
  typename Map::mapped_type const* operator->() const { return &it_->first; }

private:
  typename Map::const_iterator it_;
};

template<typename Map>
Range<MapKeyIterator<Map>>
GetValuesRange(Map& map)
{
  return { MapKeyIterator(map.begin()), MapKeyIterator(map.end()) };
}

template<typename Map>
Range<ConstMapKeyIterator<Map>>
GetValues(const Map& map)
{
  return { ConstMapKeyIterator(map.begin()), ConstMapKeyIterator(map.end()) };
}

template<typename T, typename... Args>
bool IsAny(const T& v, const Args&... args)
{
  return ((v == args) || ...);
}
