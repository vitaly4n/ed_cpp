#include "request_json.h"

#include "utils.h"

#include <algorithm>

using namespace std;

namespace {

Json::ModifyRequestPtr
CreateModifyRequest(Json::Request::Type type)
{
  using namespace Json;
  switch (type) {
    case Request::Type::ADD_BUS:
      return make_unique<AddBusRequest>();
    case Request::Type::ADD_STOP:
      return make_unique<AddStopRequest>();
    default:
      break;
  }
  return nullptr;
}

Json::ReadRequestPtr
CreateReadRequest(Json::Request::Type type)
{
  using namespace Json;
  switch (type) {
    case Request::Type::GET_BUS:
      return make_unique<GetBusRequest>();
    case Request::Type::GET_STOP:
      return make_unique<GetStopRequest>();
    default:
      break;
  }
  return nullptr;
}

} // namespace

namespace Json {

Json::ModifyRequestPtr
ParseModifyRequest(const Node& node)
{
  const auto& obj = node.AsMap();
  const auto& type_str = obj.at("type").AsString();

  static map<string, Request::Type> str2type = {
    { "Bus", Request::Type::ADD_BUS }, { "Stop", Request::Type::ADD_STOP }
  };

  auto it = str2type.find(type_str);
  if (it == end(str2type)) {
    return nullptr;
  }
  auto request = CreateModifyRequest(it->second);
  if (request) {
    request->Parse(node);
  }
  return request;
}

vector<ModifyRequestPtr>
ParseModifyRequests(const Node& node)
{
  const auto& obj = node.AsMap();
  auto it = obj.find("base_requests");
  if (it == end(obj)) {
    return {};
  }

  const auto& request_nodes = it->second.AsArray();

  vector<ModifyRequestPtr> requests;
  requests.reserve(request_nodes.size());
  for (const auto& request_node : request_nodes) {
    auto request = ParseModifyRequest(request_node);
    if (request.get()) {
      requests.push_back(move(request));
    }
  }
  return requests;
}

Json::ReadRequestPtr
ParseReadRequest(const Node& node)
{
  const auto& obj = node.AsMap();
  const auto& type_str = obj.at("type").AsString();

  static map<string, Request::Type> str2type = {
    { "Bus", Request::Type::GET_BUS }, { "Stop", Request::Type::GET_STOP }
  };

  auto it = str2type.find(type_str);
  if (it == end(str2type)) {
    return nullptr;
  }
  auto request = CreateReadRequest(it->second);
  if (request) {
    request->Parse(node);
  }
  return request;
}

std::vector<Json::ReadRequestPtr>
ParseReadRequests(const Node& node)
{
  const auto& obj = node.AsMap();
  auto it = obj.find("stat_requests");
  if (it == end(obj)) {
    return {};
  }

  const auto& request_nodes = it->second.AsArray();

  vector<ReadRequestPtr> requests;
  requests.reserve(request_nodes.size());
  for (const auto& request_node : request_nodes) {
    auto request = ParseReadRequest(request_node);
    if (request) {
      requests.push_back(move(request));
    }
  }
  return requests;
}

void
RequestsHandler::Parse(const Node& node)
{
  read_requests_ = ParseReadRequests(node);
  modify_requests_ = ParseModifyRequests(node);
}

Node
RequestsHandler::Process() const
{
  TransportManager tm;
  for (const auto& modify_request : modify_requests_) {
    modify_request->Process(tm);
  }

  vector<Node> res;
  res.reserve(read_requests_.size());
  for (const auto& read_request : read_requests_) {
    res.push_back(read_request->Process(tm));
  }
  return Node(move(res));
}

void
AddBusRequest::Parse(const Node& node)
{
  const auto& obj = node.AsMap();
  const auto& stop_nodes = obj.at("stops").AsArray();

  is_roundtrip_ = obj.at("is_roundtrip").AsBool();
  bus_ = obj.at("name").AsString();
  stops_.reserve(stop_nodes.size());
  for (const auto& stop_node : stop_nodes) {
    stops_.push_back(stop_node.AsString());
  }
}

void
AddBusRequest::Process(TransportManager& tm) const
{
  auto stops = stops_;
  if (!is_roundtrip_ && stops.size() > 1) {
    stops.insert(end(stops), next(rbegin(stops_)), rend(stops_));
  }

  tm.add_bus_route(bus_, stops);
}

void
AddStopRequest::Parse(const Node& node)
{
  const auto& obj = node.AsMap();
  stop_name_ = obj.at("name").AsString();
  latitude_ = obj.at("latitude").AsDouble();
  longitude_ = obj.at("longitude").AsDouble();

  auto road_distances_it = obj.find("road_distances");
  if (road_distances_it != end(obj) &&
      road_distances_it->second.GetType() == Json::Node::eMap) {
    for (const auto& [stop_name, dist_node] :
         road_distances_it->second.AsMap()) {
      record_.emplace_back(stop_name, dist_node.AsDouble());
    }
  }
}

void
AddStopRequest::Process(TransportManager& tm) const
{
  tm.add_stop(stop_name_, latitude_, longitude_, record_);
}

void
GetBusRequest::Parse(const Node& node)
{
  const auto& obj = node.AsMap();
  bus_ = obj.at("name").AsString();
  id_ = obj.at("id").AsInt();
}

Node
GetBusRequest::Process(const TransportManager& tm) const
{
  using DistanceType = TransportManager::DistanceType;

  map<string, Node> obj;

  auto total_num = tm.get_total_stop_num(bus_);
  auto unique_num = tm.get_unique_stops_num(bus_);
  auto route_length_straight = tm.get_route_length(bus_, DistanceType::GEO);
  auto route_length_roads = tm.get_route_length(bus_, DistanceType::ROADS);

  obj.emplace("request_id", Node(id_));
  if (total_num && unique_num && route_length_roads && route_length_straight) {
    obj.emplace("stop_count", Node(int(*total_num)));
    obj.emplace("unique_stop_count", Node(int(*unique_num)));
    obj.emplace("route_length", Node(*route_length_roads));
    obj.emplace("curvature",
                Node(*route_length_roads / *route_length_straight));
  } else {
    obj.emplace("error_message", Node(string("not found")));
  }
  return Node(move(obj));
}

void
GetStopRequest::Parse(const Node& node)
{
  const auto& obj = node.AsMap();
  stop_ = obj.at("name").AsString();
  id_ = obj.at("id").AsInt();
}

Node
GetStopRequest::Process(const TransportManager& tm) const
{
  map<string, Node> obj;
  obj.emplace("request_id", Node(id_));

  const auto bus_list = tm.get_stop_schedule(stop_);
  if (bus_list) {
    vector<Node> bus_nodes;
    bus_nodes.reserve(bus_list->size());
    for (auto& bus : *bus_list) {
      bus_nodes.push_back(Node(move(bus)));
    }
    obj.emplace("buses", Node(move(bus_nodes)));
  } else {
    obj.emplace("error_message", Node(string("not found")));
  }

  return Node(move(obj));
}

} // namespace Json
