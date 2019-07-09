#include "transport_manager.h"

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
    for (const auto& stop_id : it->second.Reversed()) {
      route.push_back(stops_.find(stop_id).operator->());
    }
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
  size_t graph_size = 0;
  std::set<StopId> unique_stops;
  for (const auto& [bus, route] : routes) {
    graph_size += route.size();
    if (!route.IsRoundtrip() && route.size() > 1) {
      graph_size += route.size();
    }
    for (const auto& stop : route) {
      if (unique_stops.emplace(stop).second) {
        ++graph_size;
      }
    }
  }

  NodesIndex nodes_data;
  nodes_data.reserve(graph_size);
  StopNodesInvIndex inv_stop_idx;
  size_t cur_node = 0;
  for (auto& stop : unique_stops) {
    StopNodes stop_nodes;
    stop_nodes.waiting_ = cur_node;
    inv_stop_idx[stop] = move(stop_nodes);

    GraphNode graph_node;
    graph_node.stop_ = move(stop);
    graph_node.type_ = GraphNode::Type::WAIT;
    nodes_data.push_back(move(graph_node));
    ++cur_node;
  }
  unique_stops.clear();

  auto add_route_nodes =
    [&](const auto& bus, bool is_backward, auto first, auto last) {
      for (auto stop_it = first; stop_it != last; ++stop_it) {
        const auto& stop = *stop_it;
        inv_stop_idx[stop].departures_.insert(cur_node);

        GraphNode graph_node;
        graph_node.bus_ = bus;
        graph_node.stop_ = stop;
        graph_node.type_ = GetNodeType(bus, is_backward, first, last, stop_it);
        graph_node.next_ = next(stop_it) != last ? *next(stop_it) : StopId{};
        nodes_data.push_back(move(graph_node));
        ++cur_node;
      }
    };
  for (const auto& [bus, route] : routes) {
    add_route_nodes(bus, false, begin(route), end(route));
    if (!route.IsRoundtrip()) {
      add_route_nodes(bus, true, rbegin(route), rend(route));
    }
  }

  assert(cur_node == graph_size);
  MathGraph graph(graph_size);

  auto add_route_edges =
    [&](const auto& bus, bool is_backward, auto first, auto last) {
      for (auto it = first; it != last && next(it) != last; ++it) {
        const auto& cur_stop_it = it;
        const auto& next_stop_it = next(it);

        auto find_node = [&](const auto& stop_it) -> Graph::VertexId {
          auto node_type = GetNodeType(bus, is_backward, first, last, stop_it);
          for (const auto& stop_node : inv_stop_idx[*stop_it].departures_) {
            const auto& node_data = nodes_data[stop_node];
            if (node_data.type_ == node_type && node_data.bus_ == bus) {
              if ((next(stop_it) == last && node_data.next_.empty()) ||
                  (next(stop_it) != last && node_data.next_ == *next(stop_it)))
                return stop_node;
            }
          }
          return 0;
        };

        Graph::Edge<double> edge;
        edge.from = find_node(cur_stop_it);
        edge.to = find_node(next_stop_it);
        edge.weight = 60 * distances.at(*cur_stop_it).at(*next_stop_it) /
                      (bus_speed * 1'000);

        graph.AddEdge(edge);
      }
    };
  for (const auto& [bus, route] : routes) {
    add_route_edges(bus, false, begin(route), end(route));
    if (!route.IsRoundtrip()) {
      add_route_edges(bus, true, rbegin(route), rend(route));
    }
  }

  for (const auto& [stop, stop_nodes] : inv_stop_idx) {
    for (const auto& departure_node : stop_nodes.departures_) {
      Graph::Edge<double> unload_edge;
      unload_edge.from = departure_node;
      unload_edge.to = stop_nodes.waiting_;
      unload_edge.weight = 0.;
      graph.AddEdge(unload_edge);

      Graph::Edge<double> load_edge;
      load_edge.from = stop_nodes.waiting_;
      load_edge.to = departure_node;
      load_edge.weight = stop_wait_time;
      graph.AddEdge(load_edge);
    }
  }

  unique_ptr<TransportGraph> res(new TransportGraph(move(graph)));
  res->inv_stop_idx_ = inv_stop_idx;
  res->nodes_data_ = nodes_data;
  return res;
}

optional<RouteStats>
TransportGraph::GetRouteStats(const StopId& from, const StopId& to) const
{
  auto stop_nodes_from_it = inv_stop_idx_.find(from);
  auto stop_nodes_to_it = inv_stop_idx_.find(to);
  if (stop_nodes_from_it == end(inv_stop_idx_) ||
      stop_nodes_to_it == end(inv_stop_idx_)) {
    return nullopt;
  }

  const auto& node_from = stop_nodes_from_it->second.waiting_;
  const auto& node_to = stop_nodes_to_it->second.waiting_;

  const auto route_info = router_.BuildRoute(node_from, node_to);
  if (!route_info) {
    return nullopt;
  }

  RouteStats res;
  res.time_ = route_info->weight;

  bool on_bus = false;
  for (auto edge_idx = decltype(route_info->edge_count){ 0 };
       edge_idx < route_info->edge_count;
       ++edge_idx) {
    auto edge_id = router_.GetRouteEdge(route_info->id, edge_idx);
    auto edge = graph_.GetEdge(edge_id);

    const auto& node_to_data = nodes_data_[edge.to];
    const auto& node_from_data = nodes_data_[edge.from];

    if (node_to_data.bus_.empty()) {
      on_bus = false;
      continue;
    }

    if (node_to_data.type_ == GraphNode::Type::WAIT) {
      on_bus = false;
    } else if (node_from_data.type_ == GraphNode::Type::WAIT) {
      on_bus = false;
      res.activities_.emplace_back(
        StopActivity{ node_to_data.stop_, edge.weight });
    } else if (on_bus) {
      auto& bus_activity = std::get<BusActivity>(res.activities_.back());
      bus_activity.time_ += edge.weight;
      bus_activity.span_ += 1;
    } else {
      on_bus = true;
      res.activities_.emplace_back(
        BusActivity{ node_to_data.bus_, 1, edge.weight });
    }
  }

  return res;
}

TransportGraph::TransportGraph(MathGraph&& graph)
  : graph_(move(graph))
  , router_(graph_)
{}
