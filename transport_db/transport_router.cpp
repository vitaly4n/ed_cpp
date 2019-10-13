#include "transport_router.h"

#include "pb_utils.h"

#include "transport_catalog.pb.h"

using namespace std;

TransportRouter::TransportRouter(const Descriptions::StopsDict& stops_dict,
                                 const Descriptions::BusesDict& buses_dict,
                                 const Json::Dict& routing_settings_json)
  : routing_settings_(MakeRoutingSettings(routing_settings_json))
{
  const size_t vertex_count = stops_dict.size() * 2;
  vertices_info_.resize(vertex_count);
  graph_ = BusGraph(vertex_count);

  FillGraphWithStops(stops_dict);
  FillGraphWithBuses(stops_dict, buses_dict);

  router_ = std::make_unique<Router>(graph_);
}

void TransportRouter::Serialize(transport_db::TransportRouter& db_transport_router) const
{
  auto& db_route_settings = *db_transport_router.mutable_routing_settings();
  {
    db_route_settings.set_bus_velocity(routing_settings_.bus_velocity);
    db_route_settings.set_bus_wait_time(routing_settings_.bus_wait_time);
  }

  auto& db_graph = *db_transport_router.mutable_graph();
  {
    auto graph_data = graph_.GetSerializationData();

    db_graph.mutable_edges()->Reserve(int(graph_data.edges_.size()));
    for (const auto& edges_data : graph_data.edges_) {
      auto& db_graph_edge = *db_graph.add_edges();
      db_graph_edge.set_to(unsigned(edges_data.to));
      db_graph_edge.set_from(unsigned(edges_data.from));
      db_graph_edge.set_weight(edges_data.weight);
    }

    db_graph.mutable_incidence_list()->Reserve(int(graph_data.incidence_lists_.size()));
    for (const auto& incidence : graph_data.incidence_lists_) {
      auto& db_graph_incidence = *db_graph.add_incidence_list();
      db_graph_incidence.mutable_edges()->Reserve(int(incidence.size()));
      for (const auto& coedge : incidence) {
        db_graph_incidence.add_edges(unsigned(coedge));
      }
    }
  }

  auto& db_router = *db_transport_router.mutable_router();
  {
    auto router_data = router_->GetSerializationData();
    db_router.mutable_entries()->Reserve(int(router_data.data_.size()));
    for (const auto& router_vertex_data : router_data.data_) {
      auto& db_vertex_data = *db_router.add_entries();
      db_vertex_data.mutable_entries()->Reserve(int(router_vertex_data.size()));
      for (const auto& router_edge_data : router_vertex_data) {
        auto& db_edge_data = *db_vertex_data.add_entries();
        db_edge_data.set_is_set(router_edge_data.is_set_);
        db_edge_data.set_weight(router_edge_data.weight_);
        db_edge_data.set_prev_edge(router_edge_data.prev_edge_);
      }
    }
  }

  auto& db_stop_vertex_ids = *db_transport_router.mutable_stop_vertex_ids();
  db_stop_vertex_ids.Reserve(int(stops_vertex_ids_.size()));
  for (const auto& [stop_name, vertex_ids] : stops_vertex_ids_) {
    auto& db_vertex_data = *db_stop_vertex_ids.Add();
    db_vertex_data.set_stop_name(stop_name);
    db_vertex_data.set_in(unsigned(vertex_ids.in));
    db_vertex_data.set_out(unsigned(vertex_ids.out));
  }

  auto& db_vertices_info = *db_transport_router.mutable_vertices_info();
  db_vertices_info.Reserve(int(vertices_info_.size()));
  for (const auto& vertex_info : vertices_info_) {
    auto& db_vertex_info = *db_vertices_info.Add();
    db_vertex_info.set_stop_name(vertex_info.stop_name);
  }

  auto& db_edges_info = *db_transport_router.mutable_edges_info();
  db_edges_info.Reserve(int(edges_info_.size()));
  for (const auto& edge_info : edges_info_) {
    auto& db_edge_info = *db_edges_info.Add();
    visit(
      [&db_edge_info](const auto& edge_info_val) {
        using info_type = remove_cv_t<remove_reference_t<decltype(edge_info_val)>>;
        if constexpr (is_same<info_type, BusEdgeInfo>::value) {
          db_edge_info.set_is_wait(false);
          db_edge_info.set_span(unsigned(edge_info_val.span_count));
          db_edge_info.set_bus_name(edge_info_val.bus_name);
        } else {
          db_edge_info.set_is_wait(true);
        }
      },
      edge_info);
  }
}

void TransportRouter::Deserialize(const transport_db::TransportRouter& db_transport_router)
{
  const auto& db_routing_settings = db_transport_router.routing_settings();
  {
    routing_settings_.bus_velocity = db_routing_settings.bus_velocity();
    routing_settings_.bus_wait_time = db_routing_settings.bus_wait_time();
  }

  const auto& db_graph = db_transport_router.graph();
  {
    BusGraph::SerializationData graph_data;
    graph_data.edges_.reserve(db_graph.edges_size());
    for (const auto& db_edge_data : db_graph.edges()) {
      Graph::Edge<double> edge{ .from = db_edge_data.from(), .to = db_edge_data.to(), .weight = db_edge_data.weight() };
      graph_data.edges_.push_back(edge);
    }
    graph_data.incidence_lists_.reserve(db_graph.incidence_list_size());
    for (const auto& db_incidence : db_graph.incidence_list()) {
      graph_data.incidence_lists_.push_back({});
      graph_data.incidence_lists_.back().reserve(db_incidence.edges_size());
      for (const auto& db_edge : db_incidence.edges()) {
        graph_data.incidence_lists_.back().push_back(db_edge);
      }
    }
    graph_ = BusGraph(move(graph_data));
  }

  const auto& db_router = db_transport_router.router();
  {
    Router::SerializationData router_data;
    router_data.data_.reserve(db_router.entries_size());
    for (const auto& db_vertex_data : db_router.entries()) {
      router_data.data_.push_back({});
      router_data.data_.back().reserve(db_vertex_data.entries_size());
      for (const auto& db_edge_data : db_vertex_data.entries()) {
        Router::SerializationData::Entry entry{ .is_set_ = db_edge_data.is_set(),
                                                .weight_ = db_edge_data.weight(),
                                                .prev_edge_ = db_edge_data.prev_edge() };
        router_data.data_.back().push_back(entry);
      }
    }
    router_ = make_unique<Router>(graph_, move(router_data));
  }

  const auto& db_stops_vertex_ids = db_transport_router.stop_vertex_ids();
  for (const auto& db_stop_vertex_ids : db_stops_vertex_ids) {
    StopVertexIds ids{ .in = db_stop_vertex_ids.in(), .out = db_stop_vertex_ids.out() };
    stops_vertex_ids_[db_stop_vertex_ids.stop_name()] = ids;
  }

  const auto& db_vertices_info = db_transport_router.vertices_info();
  vertices_info_.reserve(db_vertices_info.size());
  for (const auto& db_vertex_info : db_vertices_info) {
    vertices_info_.push_back(VertexInfo{ db_vertex_info.stop_name() });
  }

  const auto& db_edges_info = db_transport_router.edges_info();
  edges_info_.reserve(db_edges_info.size());
  for (const auto& db_edge_info : db_edges_info) {
    if (db_edge_info.is_wait()) {
      edges_info_.push_back(WaitEdgeInfo{});
    } else {
      edges_info_.push_back(BusEdgeInfo{ .bus_name = db_edge_info.bus_name(), .span_count = db_edge_info.span() });
    }
  }
}

TransportRouter::RoutingSettings
TransportRouter::MakeRoutingSettings(const Json::Dict& json)
{
  return {
    json.at("bus_wait_time").AsInt(),
    json.at("bus_velocity").AsDouble(),
  };
}

void
TransportRouter::FillGraphWithStops(const Descriptions::StopsDict& stops_dict)
{
  Graph::VertexId vertex_id = 0;

  for (const auto& [stop_name, _] : stops_dict) {
    auto& vertex_ids = stops_vertex_ids_[stop_name];
    vertex_ids.in = vertex_id++;
    vertex_ids.out = vertex_id++;
    vertices_info_[vertex_ids.in] = { stop_name };
    vertices_info_[vertex_ids.out] = { stop_name };

    edges_info_.push_back(WaitEdgeInfo{});
    const Graph::EdgeId edge_id =
      graph_.AddEdge({ vertex_ids.out, vertex_ids.in, static_cast<double>(routing_settings_.bus_wait_time) });
    assert(edge_id == edges_info_.size() - 1);
  }

  assert(vertex_id == graph_.GetVertexCount());
}

void
TransportRouter::FillGraphWithBuses(const Descriptions::StopsDict& stops_dict,
                                    const Descriptions::BusesDict& buses_dict)
{
  for (const auto& [_, bus_item] : buses_dict) {
    const auto& bus = *bus_item;
    const size_t stop_count = bus.stops.size();
    if (stop_count <= 1) {
      continue;
    }
    auto compute_distance_from = [&stops_dict, &bus](size_t lhs_idx) {
      return Descriptions::ComputeStopsDistance(*stops_dict.at(bus.stops[lhs_idx]),
                                                *stops_dict.at(bus.stops[lhs_idx + 1]));
    };
    for (size_t start_stop_idx = 0; start_stop_idx + 1 < stop_count; ++start_stop_idx) {
      const Graph::VertexId start_vertex = stops_vertex_ids_[bus.stops[start_stop_idx]].in;
      int total_distance = 0;
      for (size_t finish_stop_idx = start_stop_idx + 1; finish_stop_idx < stop_count; ++finish_stop_idx) {
        total_distance += compute_distance_from(finish_stop_idx - 1);
        edges_info_.push_back(BusEdgeInfo{
          .bus_name = bus.name,
          .span_count = finish_stop_idx - start_stop_idx,
        });
        const Graph::EdgeId edge_id = graph_.AddEdge({
          start_vertex,
          stops_vertex_ids_[bus.stops[finish_stop_idx]].out,
          total_distance * 1.0 / (routing_settings_.bus_velocity * 1000.0 / 60) // m / (km/h * 1000 / 60) = min
        });
        assert(edge_id == edges_info_.size() - 1);
      }
    }
  }
}

optional<TransportRouter::RouteInfo>
TransportRouter::FindRoute(const string& stop_from, const string& stop_to) const
{
  const Graph::VertexId vertex_from = stops_vertex_ids_.at(stop_from).out;
  const Graph::VertexId vertex_to = stops_vertex_ids_.at(stop_to).out;
  const auto route = router_->BuildRoute(vertex_from, vertex_to);
  if (!route) {
    return nullopt;
  }

  RouteInfo route_info = { .stop_from = stop_from, .stop_to = stop_to, .total_time = route->weight };
  route_info.items.reserve(route->edge_count);
  for (size_t edge_idx = 0; edge_idx < route->edge_count; ++edge_idx) {
    const Graph::EdgeId edge_id = router_->GetRouteEdge(route->id, edge_idx);
    const auto& edge = graph_.GetEdge(edge_id);
    const auto& edge_info = edges_info_[edge_id];
    if (holds_alternative<BusEdgeInfo>(edge_info)) {
      const BusEdgeInfo& bus_edge_info = get<BusEdgeInfo>(edge_info);
      route_info.items.push_back(RouteInfo::BusItem{
        .bus_name = bus_edge_info.bus_name,
        .time = edge.weight,
        .span_count = bus_edge_info.span_count,
      });
    } else {
      const Graph::VertexId vertex_id = edge.from;
      route_info.items.push_back(RouteInfo::WaitItem{
        .stop_name = vertices_info_[vertex_id].stop_name,
        .time = edge.weight,
      });
    }
  }

  // Releasing in destructor of some proxy object would be better,
  // but we do not expect exceptions in normal workflow
  router_->ReleaseRoute(route->id);
  return std::move(route_info);
}
