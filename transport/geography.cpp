#include "geography.h"

constexpr auto EARTH_RADIUS = 6'371'000.;
constexpr auto PI = 3.1415926535;

double
compute_distance(const EarthCoords& from, const EarthCoords& to)
{
  using std::acos;
  using std::cos;
  using std::sin;

  auto to_rads = [](const auto angle) { return angle * PI / 180.; };
  const auto lat_rad_1 = to_rads(from.latitude());
  const auto lon_rad_1 = to_rads(from.longitude());
  const auto lat_rad_2 = to_rads(to.latitude());
  const auto lon_rad_2 = to_rads(to.longitude());

  return EARTH_RADIUS * acos(sin(lat_rad_1) * sin(lat_rad_2) +
                             cos(lat_rad_1) * cos(lat_rad_2) *
                               cos(fabs(lon_rad_2 - lon_rad_1)));
}
