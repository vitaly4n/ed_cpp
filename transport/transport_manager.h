#pragma once

#include "graph.h"
#include "router.h"
#include "stop.h"

#include <map>
#include <memory>
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
    : stops_(first, last)
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

struct Activity : public std::variant<StopActivity, BusActivity>
{
  using variant::variant;

  enum Type
  {
    STOP = 0,
    BUS = 1
  };

  Type GetType() const { return static_cast<Type>(index()); }
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
    enum class Type
    {
      ARRIVAL = 0,
      DEPARTURE = 1
    } type_;
    StopId stop_;
  };

  struct StopNodes
  {
    Graph::VertexId arrival_;
    Graph::VertexId departure_;
  };

  using NodesIndex = std::unordered_map<Graph::VertexId, GraphNode>;
  using EdgesIndex = std::unordered_map<Graph::EdgeId, Activity>;
  using StopNodesInvIndex = std::unordered_map<StopId, StopNodes>;

private:
  TransportGraph(MathGraph&& graph);

private:
  const MathGraph graph_;
  MathRouter router_;

  NodesIndex nodes_index_;
  EdgesIndex edges_index_;
  StopNodesInvIndex inv_stop_index_;
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

  void AddStop(StopId stop_id,
                double latitude,
                double longitude,
                const DistanceTableRecord& record = {});

  void AddBusRoute(BusId bus_id,
                     std::vector<StopId> route,
                     bool is_roundtrip);

  bool IsBusDefined(const BusId& bus_id) const;
  bool IsStopDefined(const StopId& stop_id) const;

  std::optional<std::size_t> GetTotalStopNum(const BusId& bus_id) const;
  std::optional<std::size_t> GetUniqueStopNum(const BusId& bus_id) const;
  std::optional<double> GetRouteLength(const BusId& bus_id,
                                         DistanceType dt) const;

  std::optional<std::vector<StopId>> GetStopSchedule(
    const StopId& stop_id) const;

  std::optional<RouteStats> GetRouteStats(const StopId& from,
                                            const StopId& to) const;

  void InitGraph();

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
