#pragma once

#include <cmath>

constexpr auto EARTH_RADIUS = 6'371'000.;
constexpr auto PI = 3.1415926535;

class EarthCoords
{
public:
  EarthCoords() = default;

  EarthCoords(double latitude, double longitude)
    : latitude_{ latitude }
    , longitude_{ longitude }
  {}

  inline double latitude() const { return latitude_; }
  inline double longitude() const { return longitude_; }

private:
  double latitude_ = 0.;
  double longitude_ = 0.;
};

double
compute_distance(const EarthCoords& from, const EarthCoords& to);