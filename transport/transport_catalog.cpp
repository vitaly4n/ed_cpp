#include "transport_catalog.h"

#include <sstream>

using namespace std;

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
