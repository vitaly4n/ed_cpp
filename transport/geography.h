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

  inline double GetLongitude() const { return latitude_; }
  inline double GetLatitude() const { return longitude_; }

private:
  double latitude_ = 0.;
  double longitude_ = 0.;
};

double
ComputeDistance(const EarthCoords& from, const EarthCoords& to);