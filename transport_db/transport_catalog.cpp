#include "transport_catalog.h"

#include "transport_catalog.pb.h"

#include <sstream>
#include <fstream>

using namespace std;

transport_db::Stop StopToPB(const Responses::Stop& stop)
{
  transport_db::Stop res;
  for (const auto& bus : stop.bus_names) {
    res.add_buses(bus);
  }
  return res;
}

transport_db::Bus BusToPB(const Responses::Bus& bus)
{
  transport_db::Bus res;
  res.set_stop_count(bus.stop_count);
  res.set_unique_stop_count(bus.unique_stop_count);
  res.set_road_route_length(bus.road_route_length);
  res.set_geo_route_length(bus.geo_route_length);
  return res;
}

Responses::Stop StopFromPB(const transport_db::Stop& stop)
{
  Responses::Stop res;
  for (const auto& bus : stop.buses()) {
    res.bus_names.insert(bus);
  }
  return res;
}

Responses::Bus BusFromPB(const transport_db::Bus& bus)
{
  Responses::Bus res;
  res.stop_count = bus.stop_count();
  res.unique_stop_count = bus.unique_stop_count();
  res.road_route_length = bus.road_route_length();
  res.geo_route_length = bus.geo_route_length();
  return res;
}

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

void TransportCatalog::Serialize(const Json::Dict& serialization_settings) const
{
  transport_db::TransportCatalog db_catalog;

  auto& db_stops = *db_catalog.mutable_stops();
  for (const auto& [stop_name, stop] : stops_) {
    db_stops[stop_name] = StopToPB(stop);
  }
  auto& db_buses = *db_catalog.mutable_buses();
  for (const auto& [bus_name, bus] : buses_) {
    db_buses[bus_name] = BusToPB(bus);
  }

  const auto& file = serialization_settings.at("file").AsString();
  fstream output(file, ios::out | ios::trunc | ios::binary);
  db_catalog.SerializeToOstream(&output);
}

TransportCatalog TransportCatalog::Deserialize(const Json::Dict& serialization_settings)
{
  const auto& file = serialization_settings.at("file").AsString();
  fstream input(file, ios::in | ios::binary);

  transport_db::TransportCatalog db_catalog;
  db_catalog.ParseFromIstream(&input);

  // TODO;
  return TransportCatalog({}, {}, {});
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
