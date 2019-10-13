#include "transport_catalog.h"

#include "transport_catalog.pb.h"

#include "pb_utils.h"
#include "svg_renderer.h"

#include <fstream>
#include <sstream>

using namespace std;

namespace {

transport_db::Stop
StopToPB(const string& name, const Responses::Stop& stop)
{
  transport_db::Stop res;
  res.set_name(name);
  for (const auto& bus : stop.bus_names) {
    res.add_buses(bus);
  }
  return res;
}

transport_db::Bus
BusToPB(const string& name, const Responses::Bus& bus)
{
  transport_db::Bus res;
  res.set_name(name);
  res.set_stop_count(unsigned(bus.stop_count));
  res.set_unique_stop_count(unsigned(bus.unique_stop_count));
  res.set_road_route_length(bus.road_route_length);
  res.set_geo_route_length(bus.geo_route_length);
  return res;
}

pair<string, Responses::Stop>
StopFromPB(const transport_db::Stop& stop)
{
  Responses::Stop res;
  for (const auto& bus : stop.buses()) {
    res.bus_names.insert(bus);
  }
  return { stop.name(), res };
}

pair<string, Responses::Bus>
BusFromPB(const transport_db::Bus& bus)
{
  Responses::Bus res;
  res.stop_count = bus.stop_count();
  res.unique_stop_count = bus.unique_stop_count();
  res.road_route_length = bus.road_route_length();
  res.geo_route_length = bus.geo_route_length();
  return { bus.name(), res };
}

} // namespace

TransportCatalog::TransportCatalog(vector<Descriptions::InputQuery> data,
                                   const Json::Dict& routing_settings_json,
                                   unique_ptr<MapRenderer> renderer)
  : renderer_(move(renderer))
{
  auto stops_end =
    partition(begin(data), end(data), [](const auto& item) { return holds_alternative<Descriptions::Stop>(item); });

  map<string, Descriptions::Stop> stops_map;
  map<string, Descriptions::Bus> buses_map;

  Descriptions::StopsDict stops_dict;
  for (const auto& item : Range{ begin(data), stops_end }) {
    const auto& stop = get<Descriptions::Stop>(item);
    stops_dict[stop.name] = &stop;
    stops_.insert({ stop.name, {} });
    stops_map.emplace(stop.name, stop);
  }

  Descriptions::BusesDict buses_dict;
  for (const auto& item : Range{ stops_end, end(data) }) {
    const auto& bus = get<Descriptions::Bus>(item);

    buses_dict[bus.name] = &bus;
    buses_[bus.name] = Bus{ bus.stops.size(),
                            ComputeUniqueItemsCount(AsRange(bus.stops)),
                            ComputeRoadRouteLength(bus.stops, stops_dict),
                            ComputeGeoRouteDistance(bus.stops, stops_dict) };

    for (const string& stop_name : bus.stops) {
      stops_.at(stop_name).bus_names.insert(bus.name);
    }

    buses_map.emplace(bus.name, bus);
  }

  if (renderer_) {
    renderer_->Init(std::move(stops_map), std::move(buses_map));
  }
  router_ = make_unique<TransportRouter>(stops_dict, buses_dict, routing_settings_json);
}

const TransportCatalog::Stop*
TransportCatalog::GetStop(const string& name) const
{
  return GetValuePointer(stops_, name);
}

const TransportCatalog::Bus*
TransportCatalog::GetBus(const string& name) const
{
  return GetValuePointer(buses_, name);
}

optional<TransportCatalog::Route>
TransportCatalog::FindRoute(const string& stop_from, const string& stop_to) const
{
  auto route = router_->FindRoute(stop_from, stop_to);
  if (!route) {
    return nullopt;
  }
  string route_map;
  if (renderer_) {
    route_map = renderer_->RenderRoute(*route);
  }
  return TransportCatalog::Route{ std::move(*route), std::move(route_map) };
}

string
TransportCatalog::RenderMap() const
{
  if (!renderer_) {
    return {};
  }

  return renderer_->Render();
}

TransportCatalog
TransportCatalog::Deserialize(const Json::Dict& serialization_settings)
{
  const auto& file = serialization_settings.at("file").AsString();
  ifstream input(file, ios::in | ios::binary);

  transport_db::TransportCatalog db_catalog;
  const bool read_res = db_catalog.ParseFromIstream(&input);
  assert(read_res);

  TransportCatalog res{};
  for (const auto& db_stop : db_catalog.stops()) {
    auto name2stop = StopFromPB(db_stop);
    res.stops_[move(name2stop.first)] = move(name2stop.second);
  }
  for (const auto& db_bus : db_catalog.buses()) {
    auto name2bus = BusFromPB(db_bus);
    res.buses_[move(name2bus.first)] = move(name2bus.second);
  }

  res.router_ = make_unique<TransportRouter>();
  res.router_->Deserialize(db_catalog.transport_router());

  res.renderer_ = make_unique<Svg::MapRenderer>();
  res.renderer_->Deserialize(db_catalog.transport_renderer());

  return res;
}

void
TransportCatalog::Serialize(const Json::Dict& serialization_settings) const
{
  transport_db::TransportCatalog db_catalog;

  auto& db_stops = *db_catalog.mutable_stops();
  db_stops.Reserve(int(stops_.size()));
  for (const auto& [stop_name, stop] : stops_) {
    *db_stops.Add() = StopToPB(stop_name, stop);
  }
  auto& db_buses = *db_catalog.mutable_buses();
  db_buses.Reserve(int(buses_.size()));
  for (const auto& [bus_name, bus] : buses_) {
    *db_buses.Add() = BusToPB(bus_name, bus);
  }

  // here (not only though) the dragons will be
  if (router_) {
    router_->Serialize(*db_catalog.mutable_transport_router());
  }
  if (renderer_) {
    renderer_->Serialize(*db_catalog.mutable_transport_renderer());
  }

  const auto& file = serialization_settings.at("file").AsString();
  ofstream output(file, ios::out | ios::trunc | ios::binary);
  const bool write_res = db_catalog.SerializeToOstream(&output);
  assert(write_res);
}

int
TransportCatalog::ComputeRoadRouteLength(const vector<string>& stops, const Descriptions::StopsDict& stops_dict)
{
  int result = 0;
  for (size_t i = 1; i < stops.size(); ++i) {
    result += Descriptions::ComputeStopsDistance(*stops_dict.at(stops[i - 1]), *stops_dict.at(stops[i]));
  }
  return result;
}

double
TransportCatalog::ComputeGeoRouteDistance(const vector<string>& stops, const Descriptions::StopsDict& stops_dict)
{
  double result = 0;
  for (size_t i = 1; i < stops.size(); ++i) {
    result += Sphere::Distance(stops_dict.at(stops[i - 1])->position, stops_dict.at(stops[i])->position);
  }
  return result;
}
