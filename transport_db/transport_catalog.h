#pragma once

#include "descriptions.h"
#include "json.h"
#include "transport_router.h"
#include "utils.h"

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Responses {
struct Stop
{
  std::set<std::string> bus_names;
};

struct Bus
{
  size_t stop_count = 0;
  size_t unique_stop_count = 0;
  int road_route_length = 0;
  double geo_route_length = 0.0;
};

struct Route
{
  TransportRouter::RouteInfo route_info;
  std::string route_map;
};

}

class MapRenderer
{
public:
  virtual ~MapRenderer() = default;

  virtual void Init(std::map<std::string, Descriptions::Stop> stops,
                    std::map<std::string, Descriptions::Bus> buses) = 0;

  virtual std::string Render() const = 0;
  virtual std::string RenderRoute(const TransportRouter::RouteInfo& route_info) const = 0;

  virtual void Serialize(transport_db::TransportRenderer& db_renderer) const = 0;
  virtual void Deserialize(const transport_db::TransportRenderer& db_renderer) = 0;
};

class TransportCatalog
{
private:
  using Bus = Responses::Bus;
  using Stop = Responses::Stop;
  using Route = Responses::Route;

public:
  TransportCatalog() = default;

  TransportCatalog(std::vector<Descriptions::InputQuery> data,
                   const Json::Dict& routing_settings_json,
                   std::unique_ptr<MapRenderer> renderer = nullptr);

  const Stop* GetStop(const std::string& name) const;
  const Bus* GetBus(const std::string& name) const;

  std::optional<Route> FindRoute(const std::string& stop_from, const std::string& stop_to) const;

  std::string RenderMap() const;

  void Serialize(const Json::Dict& serialization_settings) const;
  static TransportCatalog Deserialize(const Json::Dict& serialization_settings);

private:
  static int ComputeRoadRouteLength(const std::vector<std::string>& stops, const Descriptions::StopsDict& stops_dict);

  static double ComputeGeoRouteDistance(const std::vector<std::string>& stops,
                                        const Descriptions::StopsDict& stops_dict);

  std::unordered_map<std::string, Stop> stops_;
  std::unordered_map<std::string, Bus> buses_;
  std::unique_ptr<TransportRouter> router_;
  std::unique_ptr<MapRenderer> renderer_;
};
