#pragma once

#include "geography.h"

#include <string>
#include <utility>

class Stop
{
public:
  Stop(std::string name, double latitude, double longitude)
    : name_{ std::move(name) }
    , coords_{ latitude, longitude }
  {}

  const std::string& GetName() const { return name_; }
  const EarthCoords& GetCoords() const { return coords_; }

private:
  std::string name_;
  EarthCoords coords_;
};

struct StopComparator
{
  using is_transparent = std::true_type;

  bool operator()(const Stop& lhs, const Stop& rhs) const
  {
    return lhs.GetName() < rhs.GetName();
  }
  bool operator()(const std::string& lhs, const Stop& rhs) const
  {
    return lhs < rhs.GetName();
  }
  bool operator()(const Stop& lhs, const std::string& rhs) const
  {
    return lhs.GetName() < rhs;
  }
};
