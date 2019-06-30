#include "transport_manager.h"

#include <algorithm>

using namespace std;

void
TransportManager::add_stop(StopId stop_id,
                           double latitude,
                           double longitude,
                           const DistanceTableRecord& dist_table_rec)
{
  auto& dist_from_here = dist_table_[stop_id];
  for (const auto& [stop_id_to, dist] : dist_table_rec) {
    dist_from_here[stop_id_to] = dist;
    dist_table_[stop_id_to].emplace(stop_id, dist);
  }

  stop_schedules_.emplace(stop_id, BusList{});
  stops_.emplace(Stop{ move(stop_id), latitude, longitude });
}

void
TransportManager::add_bus_route(BusId bus_id, vector<StopId> route)
{
  for (const auto& stop : route) {
    stop_schedules_[stop].insert(bus_id);
  }
  bus_routes_[move(bus_id)] = move(route);
}

bool
TransportManager::is_bus_defined(const BusId& bus_id) const
{
  return bus_routes_.find(bus_id) != end(bus_routes_);
}

bool
TransportManager::is_stop_defined(const StopId& stop_id) const
{
  return stops_.find(stop_id) != end(stops_);
}

optional<size_t>
TransportManager::get_total_stop_num(const BusId& bus_id) const
{
  auto it = bus_routes_.find(bus_id);
  if (it == end(bus_routes_)) {
    return nullopt;
  }

  return it->second.size();
}

optional<size_t>
TransportManager::get_unique_stops_num(const BusId& bus_id) const
{
  auto it = bus_routes_.find(bus_id);
  if (it == end(bus_routes_)) {
    return nullopt;
  }

  const auto& route = it->second;
  vector<string_view> unique_stops{ begin(route), end(route) };
  sort(begin(unique_stops), end(unique_stops));
  return distance(begin(unique_stops),
                  unique(begin(unique_stops), end(unique_stops)));
}

optional<double>
TransportManager::get_route_length(const BusId& bus_id, DistanceType dt) const
{
  auto it = bus_routes_.find(bus_id);
  if (it == end(bus_routes_)) {
    return nullopt;
  }

  vector<const Stop*> route;
  route.reserve(it->second.size());
  for (const auto& stop_id : it->second) {
    route.push_back(stops_.find(stop_id).operator->());
  }

  auto get_distance = [&dt, this](const Stop& from, const Stop& to) {
    if (dt == DistanceType::GEO) {
      return compute_distance(from.coords(), to.coords());
    } else {
      const auto& distances_from = dist_table_.find(from.name())->second;
      auto distance_from_it = distances_from.find(to.name());
      return distance_from_it == end(distances_from)
               ? 0
               : distances_from.find(to.name())->second;
    }
  };

  auto res = 0.;
  for (auto route_it = begin(route);
       route_it != end(route) && next(route_it) != end(route);
       ++route_it) {
    if (!*route_it || !*next(route_it)) {
      continue;
    }

    const auto& cur_stop = **route_it;
    const auto& next_stop = **next(route_it);

    res += get_distance(cur_stop, next_stop);
  }
  return res;
}

optional<vector<TransportManager::StopId>>
TransportManager::get_stop_schedule(const StopId& stop_id) const
{
  auto it = stop_schedules_.find(stop_id);
  if (it == end(stop_schedules_)) {
    return nullopt;
  }

  const auto& bus_list = it->second;
  return vector<StopId>{ begin(bus_list), end(bus_list) };
}
