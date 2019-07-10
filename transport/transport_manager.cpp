#include "transport_manager.h"

#include "profile.h"

#include <algorithm>
#include <iterator>

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
TransportManager::add_bus_route(BusId bus_id,
                                vector<StopId> route,
                                bool is_roundtrip)
{
  for (const auto& stop : route) {
    stop_schedules_[stop].insert(bus_id);
  }
  bus_routes_[move(bus_id)] = Route(move(route), is_roundtrip);
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

  const auto& route = it->second;
  auto res = route.size();
  if (!route.IsRoundtrip() && route.size() > 0) {
    res += route.size() - 1;
  }

  return res;
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
  route.reserve(it->second.size() * (it->second.IsRoundtrip() ? 1 : 2));
  for (const auto& stop_id : it->second) {
    route.push_back(stops_.find(stop_id).operator->());
  }
  if (!it->second.IsRoundtrip()) {
    decltype(route) reversed_route;
    for (const auto& stop_id : it->second.Reversed()) {
      reversed_route.push_back(stops_.find(stop_id).operator->());
    }
    if (!reversed_route.empty()) {
      route.insert(
        end(route), next(begin(reversed_route)), end(reversed_route));
    }
  }

  auto get_distance = [&dt, this](const Stop& from, const Stop& to) -> double {
    if (dt == DistanceType::GEO) {
      return compute_distance(from.coords(), to.coords());
    } else {
      const auto& distances_from = dist_table_.find(from.name())->second;
      auto distance_from_it = distances_from.find(to.name());
      return distance_from_it == end(distances_from) ? 0
                                                     : distance_from_it->second;
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

std::optional<RouteStats>
TransportManager::get_route_stats(const StopId& from, const StopId& to) const
{
  if (graph_) {
    return graph_->GetRouteStats(from, to);
  } else {
    return nullopt;
  }
}

void
TransportManager::init_graph()
{
  if (!graph_) {
    graph_ = TransportGraph::Create(bus_routes_,
                                     dist_table_,
                                     settings_.bus_velocity_,
                                     double(settings_.bus_wait_time_));
  }
}

TransportManager::TransportManager() = default;

TransportManager::TransportManager(const TransportManager::Settings& settings)
  : settings_(settings)
{}

void
TransportScheduleBuilder::AddStop(const StopId& stop,
                                  double latitude,
                                  double longitude,
                                  const DistanceTableRecord& record)
{
  auto from_stop_id_view = DeclareStopId(stop);
  auto& dist_from = stops_distances_[from_stop_id_view];
  for (const auto& [to_stop_id, distance] : record) {
    auto to_stop_id_view = DeclareStopId(to_stop_id);
    dist_from[to_stop_id_view] = distance;
    stops_distances_[from_stop_id_view].emplace(to_stop_id_view, distance);
  }
  stops_coords_.emplace(from_stop_id_view, EarthCoords(latitude, longitude));
}

void
TransportScheduleBuilder::AddBus(const BusId& bus,
                                 const RouteTableRecord& record)
{
  auto bus_id_view = DeclareBusId(bus);

  auto& bus_route = buses_routes_[bus_id_view];
  for (const auto& stop : record) {
    auto stop_id_view = DeclareStopId(stop);
    bus_route.push_back(stop_id_view);
  }
}

TransportScheduleBuilder::BusIdView
TransportScheduleBuilder::DeclareBusId(const BusId& bus)
{
  auto it = bus_ids_.find(bus);
  if (it == end(bus_ids_)) {
    return *bus_ids_.emplace(bus).first;
  }
  return *it;
}

TransportScheduleBuilder::StopIdView
TransportScheduleBuilder::DeclareStopId(const StopId& stop)
{
  auto it = stop_ids_.find(stop);
  if (it == end(stop_ids_)) {
    return *stop_ids_.emplace(stop).first;
  }
  return *it;
}

TransportSchedule
TransportSchedule::create(TransportScheduleBuilder&& builder)
{
  TransportSchedule res;
  res.builder_ = move(builder);
  return res;
}

std::optional<BusStats>
TransportSchedule::GetBusStats(const BusId& bus)
{
  const auto& routes = builder_.buses_routes_;
  auto routes_it = routes.find(bus);
  if (routes_it == end(routes)) {
    return nullopt;
  }

  const auto& route = routes_it->second;

  double roads_length = 0.;
  double geo_length = 0.;
  for (auto stop_it = begin(route);
       stop_it != end(route) && next(stop_it) != end(route);
       ++stop_it) {
    const auto& from_stop_name = *stop_it;
    const auto& to_stop_name = *next(stop_it);

    roads_length +=
      builder_.stops_distances_.at(from_stop_name).at(to_stop_name);

    const auto& from = builder_.stops_coords_.at(from_stop_name);
    const auto& to = builder_.stops_coords_.at(to_stop_name);
    geo_length += compute_distance(from, to);
  }

  BusStats res;
  res.total_stop_count = route.size();
  res.unique_stop_count = set<string_view>(begin(route), end(route)).size();
  res.route_length = roads_length;
  res.curvature = (geo_length == 0.) ? roads_length / geo_length : 1.;

  bus_stats_cache_[bus] = res;
  return res;
}

std::optional<StopStats>
TransportSchedule::GetStopStats(const StopId& stop)
{
  if (builder_.stop_ids_.count(stop) == 0) {
    return nullopt;
  }

  StopStats res;
  for (const auto& [bus, route] : builder_.buses_routes_) {
    if (std::find(begin(route), end(route), stop) != end(route)) {
      res.buses_.insert(BusId(bus));
    }
  }
  return res;
}

unique_ptr<TransportGraph>
TransportGraph::Create(const BusRoutes& routes,
                        const DistanceTable& distances,
                        double bus_speed,
                        double stop_wait_time)
{
  std::set<StopId> unique_stops;
  for (const auto& [bus, route] : routes) {
    for (const auto& stop : route) {
      unique_stops.emplace(stop).second;
    }
  }

  size_t graph_size = unique_stops.size() * 2;
  MathGraph graph(graph_size);

  StopNodesInvIndex stops_inv_index;
  EdgesIndex edges_index;
  NodesIndex nodes_index;

  size_t cur_node_id = 0;
  for (auto& stop : unique_stops) {
    GraphNode departure_node;
    departure_node.type_ = GraphNode::Type::DEPARTURE;
    departure_node.stop_ = stop;
    const auto departure_node_id = cur_node_id++;

    GraphNode arrival_node;
    arrival_node.type_ = GraphNode::Type::ARRIVAL;
    arrival_node.stop_ = stop;
    const auto arrival_node_id = cur_node_id++;

    nodes_index[departure_node_id] = move(departure_node);
    nodes_index[arrival_node_id] = move(arrival_node);
    stops_inv_index[stop] = StopNodes{ arrival_node_id, departure_node_id };

    Graph::Edge<double> waiting_edge;
    waiting_edge.from = arrival_node_id;
    waiting_edge.to = departure_node_id;
    waiting_edge.weight = stop_wait_time;

    const auto waiting_edge_id = graph.AddEdge(waiting_edge);
    edges_index[waiting_edge_id] = StopActivity{ stop, stop_wait_time };
  }
  unique_stops.clear();
  assert(cur_node_id == graph_size);

  auto add_route_edges = [&](const auto& bus, auto first, auto last) {
    for (auto from_it = first; from_it != last; ++from_it) {
      size_t span = 0;
      double distance = 0.;
      for (auto to_it = next(from_it); to_it != last; ++to_it) {
        span += 1;
        distance +=
          60 * distances.at(*prev(to_it)).at(*to_it) / (bus_speed * 1'000);

        Graph::Edge<double> route_edge;
        route_edge.from = stops_inv_index.at(*from_it).departure_;
        route_edge.to = stops_inv_index.at(*to_it).arrival_;
        route_edge.weight = distance;

        const auto route_edge_id = graph.AddEdge(route_edge);
        edges_index[route_edge_id] = BusActivity{ bus, span, distance };
      }
    }
  };

  for (const auto& [bus, route] : routes) {
    add_route_edges(bus, begin(route), end(route));
    if (!route.IsRoundtrip()) {
      add_route_edges(bus, rbegin(route), rend(route));
    }
  }

  unique_ptr<TransportGraph> res(new TransportGraph(move(graph)));
  res->nodes_index_ = move(nodes_index);
  res->edges_index_ = move(edges_index);
  res->inv_stop_index_ = move(stops_inv_index);
  return res;
}

std::optional<RouteStats>
TransportGraph::GetRouteStats(const StopId& from, const StopId& to) const
{
  auto from_stop_nodes_it = inv_stop_index_.find(from);
  auto to_stop_nodes_it = inv_stop_index_.find(to);
  if (from_stop_nodes_it == end(inv_stop_index_) ||
      to_stop_nodes_it == end(inv_stop_index_)) {
    return nullopt;
  }

  const auto& from_node_id = from_stop_nodes_it->second.arrival_;
  const auto& to_node_id = to_stop_nodes_it->second.arrival_;
  const auto route_info = router_.BuildRoute(from_node_id, to_node_id);
  if (!route_info) {
    return nullopt;
  }

  RouteStats res;
  res.time_ = route_info->weight;
  for (auto edge_idx = decltype(route_info->edge_count){ 0 };
       edge_idx < route_info->edge_count;
       ++edge_idx) {
    auto edge_id = router_.GetRouteEdge(route_info->id, edge_idx);
    res.activities_.push_back(edges_index_.at(edge_id));
  }
  return res;
}

TransportGraph::TransportGraph(MathGraph&& graph)
  : graph_(move(graph))
  , router_(graph_)
{}
