#pragma once

#include "graph.h"
#include "router.h"
#include "stop.h"

#include <map>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

struct Distance
{
  double straight_ = -1.;
  double roads_ = -1.;
};

using DistanceTableRecord = std::unordered_map<std::string, double>;
using DistanceTable = std::unordered_map<std::string, DistanceTableRecord>;

using RouteTableRecord = std::vector<std::string>;
using RouteTable = std::unordered_map<std::string, RouteTableRecord>;

using BusId = std::string;
using StopId = std::string;

class Route
{
public:
  using Data = std::vector<StopId>;
  using iterator = Data::iterator;
  using const_iterator = Data::const_iterator;
  using reverse_iterator = Data::reverse_iterator;
  using const_reverse_iterator = Data::const_reverse_iterator;
  using size_type = Data::size_type;

  Route() = default;

  template<typename It>
  Route(It first, It last, bool is_roundtrip = false)
    : stops(first, last)
    , is_roundtrip_(is_roundtrip)
  {}

  Route(const Data& data, bool is_roundtrip = false)
    : stops_(data)
    , is_roundtrip_(is_roundtrip)
  {}

  Route(Data&& data, bool is_roundtrip = false)
    : stops_(move(data))
    , is_roundtrip_(is_roundtrip)
  {}

  bool IsRoundtrip() const { return is_roundtrip_; }
  auto Straight() const { return Range(begin(), end()); }
  auto Reversed() const { return Range(rbegin(), rend()); }

  iterator begin() { return std::begin(stops_); }
  const_iterator begin() const { return std::begin(stops_); }
  iterator end() { return std::end(stops_); }
  const_iterator end() const { return std::end(stops_); }

  reverse_iterator rbegin() { return std::rbegin(stops_); }
  const_reverse_iterator rbegin() const { return std::rbegin(stops_); }
  reverse_iterator rend() { return std::rend(stops_); }
  const_reverse_iterator rend() const { return std::rend(stops_); }

  size_type size() const noexcept { return stops_.size(); }
  bool empty() const noexcept { return stops_.empty(); }

private:
  Data stops_;
  bool is_roundtrip_ = false;
};

using BusRoutes = std::unordered_map<BusId, Route>;

class TransportScheduleBuilder
{
  using BusIdView = std::string_view;
  using StopIdView = std::string_view;

public:
  void AddStop(const StopId& stop,
               double latitude,
               double longitude,
               const DistanceTableRecord& record);

  void AddBus(const BusId& bus, const RouteTableRecord& record);

private:
  friend class TransportSchedule;

  BusIdView DeclareBusId(const BusId& bus);
  StopIdView DeclareStopId(const StopId& stop);

  std::unordered_set<StopId> stop_ids_;
  std::unordered_set<BusId> bus_ids_;

  using BusRoute = std::vector<StopIdView>;
  using BusRoutes = std::unordered_map<BusIdView, BusRoute>;
  BusRoutes buses_routes_;

  using DistancesFromStop = std::unordered_map<StopIdView, double>;
  using StopsDistances = std::unordered_map<StopIdView, DistancesFromStop>;
  StopsDistances stops_distances_;

  using StopsCoords = std::unordered_map<StopIdView, EarthCoords>;
  StopsCoords stops_coords_;
};

struct BusStats
{
  std::size_t total_stop_count = 0;
  std::size_t unique_stop_count = 0;
  double route_length = 0.;
  double curvature = 0.;
};

struct StopStats
{
  std::set<BusId> buses_;
};

class TransportSchedule
{
public:
  TransportSchedule create(TransportScheduleBuilder&& builder);

  std::optional<BusStats> GetBusStats(const BusId& bus);
  std::optional<StopStats> GetStopStats(const StopId& stop);

private:
  TransportSchedule() = default;

private:
  TransportScheduleBuilder builder_;

  std::unordered_map<BusId, BusStats> bus_stats_cache_;
  std::unordered_map<StopId, StopStats> stop_stats_cache_;
};

struct StopActivity
{
  StopId stop_;
  double time_ = 0.;
};

struct BusActivity
{
  BusId bus_;
  std::size_t span_ = 0;
  double time_ = 0.;
};

struct RouteStats
{
  double time_ = 0.;
  std::vector<std::variant<StopActivity, BusActivity>> activities_;
};

class TransportGraph
{
public:
  using MathGraph = Graph::DirectedWeightedGraph<double>;
  using MathRouter = Graph::Router<double>;

  TransportGraph(const TransportGraph&) = default;
  TransportGraph& operator=(const TransportGraph&) = default;

  static std::unique_ptr<TransportGraph> Create(const BusRoutes& routes,
                                                const DistanceTable& distances,
                                                double bus_speed,
                                                double stop_wait_time);

  std::optional<RouteStats> GetRouteStats(const StopId& from,
                                          const StopId& to) const;

private:
  struct GraphNode
  {
    BusId bus_;
    StopId stop_;
    StopId next_;
    enum class Type
    {
      START,
      END,
      FORWARD,
      BACKWARD,
      WAIT
    } type_;
  };

  struct StopNodes
  {
    Graph::VertexId waiting_;
    std::set<Graph::VertexId> departures_;
  };

  using NodesIndex = std::vector<GraphNode>;
  using StopNodesInvIndex = std::unordered_map<StopId, StopNodes>;

private:
  TransportGraph(MathGraph&& graph);

  template<typename It>
  static GraphNode::Type GetNodeType(const BusId& bus,
                                     bool is_backward,
                                     It first,
                                     It last,
                                     It it)
  {
    if (bus.empty()) {
      return GraphNode::Type::WAIT;
    } else if (it == first) {
      return GraphNode::Type::START;
    } else if (it == last) {
      return GraphNode::Type::START;
    } else if (it == prev(last)) {
      return GraphNode::Type::END;
    } else {
      return is_backward ? GraphNode::Type::BACKWARD : GraphNode::Type::FORWARD;
    }
  }

private:
  const MathGraph graph_;
  MathRouter router_;

  NodesIndex nodes_data_;
  StopNodesInvIndex inv_stop_idx_;
};

class TransportManager
{
public:
  using BusId = std::string;
  using StopId = std::string;

  struct Settings
  {
    double bus_velocity_ = 0.;
    std::size_t bus_wait_time_ = 0;
  };

  enum class DistanceType
  {
    ROADS,
    GEO
  };

  using DistanceTableRecord = std::vector<std::pair<std::string, double>>;

  TransportManager();
  TransportManager(const Settings& settings);

  void add_stop(StopId stop_id,
                double latitude,
                double longitude,
                const DistanceTableRecord& record = {});

  void add_bus_route(BusId bus_id,
                     std::vector<StopId> route,
                     bool is_roundtrip);

  bool is_bus_defined(const BusId& bus_id) const;
  bool is_stop_defined(const StopId& stop_id) const;

  std::optional<std::size_t> get_total_stop_num(const BusId& bus_id) const;
  std::optional<std::size_t> get_unique_stops_num(const BusId& bus_id) const;
  std::optional<double> get_route_length(const BusId& bus_id,
                                         DistanceType dt) const;

  std::optional<std::vector<StopId>> get_stop_schedule(
    const StopId& stop_id) const;

  std::optional<RouteStats> get_route_stats(const StopId& from,
                                            const StopId& to) const;

  void init_graph();

private:
  BusRoutes bus_routes_;

  using BusList = std::set<BusId>;
  using StopSchedule = std::unordered_map<StopId, BusList>;
  StopSchedule stop_schedules_;

  using StopSet = std::set<Stop, StopComparator>;
  StopSet stops_;

  using DistanceTable =
    std::unordered_map<std::string, std::unordered_map<std::string, double>>;
  DistanceTable dist_table_;

  std::unique_ptr<TransportGraph> graph_;
  Settings settings_;
};
