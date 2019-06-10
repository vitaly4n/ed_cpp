#include "test_runner.h"

#include <algorithm>
#include <numeric>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

class BusStop
{
public:
  BusStop(string name, double latitude, double longitude)
    : latitude_{ latitude }
    , longitude_{ longitude }
    , name_{ move(name) }
  {}

  double latitude() const { return latitude_; }
  double longitude() const { return longitude_; }
  const string& name() const { return name_; }

private:
  string name_;
  double latitude_ = 0.;
  double longitude_ = 0.;
};

double
compute_distance(const BusStop& from, const BusStop& to)
{
  return 0.;
}

struct BusStopComparator
{
  using is_transparent = true_type;
  bool operator()(const BusStop& lhs, const BusStop& rhs) const
  {
    return lhs.name() < rhs.name();
  }
  bool operator()(const string& lhs, const BusStop& rhs) const
  {
    return lhs < rhs.name();
  }
  bool operator()(const BusStop& lhs, const string& rhs) const
  {
    return lhs.name() < rhs;
  }
};

class TransportManager
{
public:
  using Bus = size_t;

  void add_bus_stop(BusStop&& bus_stop) { bus_stops_.emplace(move(bus_stop)); }
  void add_bus(Bus bus, vector<string> stops)
  {
    BusRoute route;
    route.reserve(stops.size());
    for (const auto& stop : stops) {
      auto it = bus_stops_.find(stop);
      if (it == end(bus_stops_)) {
        throw runtime_error("bus stop is not defined");
      }
      route.push_back(it);
    }
    bus_routes_[bus] = move(route);
  }

  size_t get_total_stops_num(Bus bus) const
  {
    auto it = bus_routes_.find(bus);
    return it != end(bus_routes_) ? it->second.size() : 0;
  }

  size_t get_unique_stops_num(Bus bus) const
  {
    auto it = bus_routes_.find(bus);
    if (it == end(bus_routes_)) {
      return 0;
    }

    vector<string_view> unique_routes;
    unique_routes.reserve(it->second.size());
    for (const auto& bus_stop_it : it->second) {
      unique_routes.push_back(bus_stop_it->name());
    }

    sort(begin(unique_routes), end(unique_routes));
    auto unique_end = unique(begin(unique_routes), end(unique_routes));
    return distance(begin(unique_routes), unique_end);
  }

  double get_route_length(Bus bus) const
  {
    auto it = bus_routes_.find(bus);
    if (it == end(bus_routes_)) {
      return 0.;
    }

    auto res = 0.;
    auto& route = it->second;
    for (auto route_it = begin(route);
         route_it != end(route) && next(route_it) != end(route);
         ++route_it) {
      res += compute_distance(**route_it, **next(route_it));
    }
    return res;
  }

private:
  using BusStopDatabase = set<BusStop, BusStopComparator>;
  BusStopDatabase bus_stops_;

  using BusRoute = vector<BusStopDatabase::iterator>;
  using BusRouteDatabase = unordered_map<Bus, BusRoute>;
  BusRouteDatabase bus_routes_;
};

#ifdef LOCAL_TEST

void
test_total_stops()
{
  TransportManager manager;
  manager.add_bus_stop(BusStop{ "yandex", 0., 0. });
  manager.add_bus_stop(BusStop{ "google", 0., 0. });
  manager.add_bus_stop(BusStop{ "binq", 0., 0. });

  manager.add_bus(1, { "yandex", "google", "yandex", "google" });
  manager.add_bus(2, { "google", "yandex", "binq", "google", "yandex" });

  ASSERT_EQUAL(manager.get_total_stops_num(1), 4);
  ASSERT_EQUAL(manager.get_total_stops_num(2), 5);
}

void
test_unique_stops()
{
  TransportManager manager;
  manager.add_bus_stop(BusStop{ "yandex", 0., 0. });
  manager.add_bus_stop(BusStop{ "google", 0., 0. });
  manager.add_bus_stop(BusStop{ "binq", 0., 0. });

  manager.add_bus(1, { "yandex", "google", "yandex", "google" });
  manager.add_bus(2, { "google", "yandex", "binq", "google", "yandex" });

  ASSERT_EQUAL(manager.get_unique_stops_num(1), 2);
  ASSERT_EQUAL(manager.get_unique_stops_num(2), 3);
}

#endif // LOCAL_TEST

int
main()
{
#ifdef LOCAL_TEST
  TestRunner tr;
  RUN_TEST(tr, test_total_stops);
  RUN_TEST(tr, test_unique_stops);

#endif // LOCAL_TEST

  return 0;
}
