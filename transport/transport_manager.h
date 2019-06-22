#pragma once

#include "stop.h"

#include <optional>
#include <set>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

class TransportManager
{
public:
  using BusId = std::string;
  using StopId = std::string;

  void add_stop(StopId stop_id, double latitude, double longitude);
  void add_bus_route(BusId bus_id, std::vector<StopId> route);

  bool is_bus_defined(const BusId& bus_id) const;
  bool is_stop_defined(const StopId& stop_id) const;

  std::optional<std::size_t> get_total_stop_num(const BusId& bus_id) const;
  std::optional<std::size_t> get_unique_stops_num(const BusId& bus_id) const;
  std::optional<double> get_route_length(const BusId& bus_id) const;

  std::optional<std::vector<StopId>> get_stop_schedule(const StopId& stop_id) const;

private:
  using Route = std::vector<StopId>;
  using BusRoutes = std::unordered_map<BusId, Route>;
  BusRoutes bus_routes_;

  using BusList = std::set<BusId>;
  using StopSchedule = std::unordered_map<StopId, BusList>;
  StopSchedule stop_schedules_;

  using StopSet = std::set<Stop, StopComparator>;
  StopSet stops_;
};
