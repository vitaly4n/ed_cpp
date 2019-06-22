#pragma once

#include <cmath>

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