#include "request_plain.h"

#include "transport_manager.h"
#include "utils.h"

#include <map>
#include <sstream>
#include <string>

using namespace std;

Request::Request(Type type)
  : type_{ type }
{}

void
WriteRequest::read(string_view data)
{
  auto [operation, operand] = SplitTwoStrict(data, ": ");
  read(operation, *operand);
}

AddBusRequest::AddBusRequest()
  : WriteRequest(Type::ADD_BUS)
{}

bool
AddBusRequest::is_one_way(string_view route_desc) const
{
  return route_desc.find(">") != route_desc.npos;
}

void
AddBusRequest::read(string_view operation, string_view operand)
{
  stops_data_ = operand;

  bus_ = operation;

  is_roundtrip_ = is_one_way(stops_data_);
  string_view separator = is_roundtrip_ ? " > " : " - ";

  const auto stops_raw = Split(stops_data_, separator);
  stops_.insert(begin(stops_), begin(stops_raw), end(stops_raw));
}

void
AddBusRequest::process(TransportManager& tm) const
{
  vector<string> stops(begin(stops_), end(stops_));
  tm.add_bus_route(bus_, stops, is_roundtrip_);
}

AddBusStopRequest::AddBusStopRequest()
  : WriteRequest(Type::ADD_STOP)
{}

void
AddBusStopRequest::read(string_view operation, string_view operand)
{
  stop_name_ = ReadToken(operation, ": ");
  latitude_ = ConvertFromView<double>(ReadToken(operand, ", "));
  longitude_ = ConvertFromView<double>(ReadToken(operand, ", "));

  auto record_data = Split(operand, ", ");
  for (auto& dist_to_data : record_data) {
    const auto to_dist =
      ConvertFromView<double>(ReadToken(dist_to_data, "m to "));
    const auto to_stop_id = dist_to_data;
    record_.emplace_back(to_stop_id, to_dist);
  }
}

void
AddBusStopRequest::process(TransportManager& tm) const
{
  tm.add_stop(stop_name_, latitude_, longitude_, record_);
}

GetBusRequest::GetBusRequest()
  : ReadRequest<string>(Type::GET_BUS)
{}

void
GetBusRequest::read(string_view data)
{
  bus_ = data;
}

string
GetBusRequest::process(const TransportManager& tm) const
{
  const auto total_num = tm.get_total_stop_num(bus_);
  const auto unique_num = tm.get_unique_stops_num(bus_);
  const auto route_length_straight =
    tm.get_route_length(bus_, TransportManager::DistanceType::GEO);
  const auto route_length_roads =
    tm.get_route_length(bus_, TransportManager::DistanceType::ROADS);

  ostringstream ss;
  ss << "Bus " << bus_ << ": ";
  if (total_num && unique_num && route_length_straight && route_length_roads) {
    const auto curvature = *route_length_roads / *route_length_straight;
    ss << *total_num << " stops on route, " << *unique_num << " unique stops, "
       << *route_length_roads << " route length, " << curvature << " curvature";
  } else {
    ss << "not found";
  }
  return ss.str();
}

GetStopRequest::GetStopRequest()
  : ReadRequest<string>(Type::GET_STOP)
{}

void
GetStopRequest::read(string_view data)
{
  stop_ = data;
}

string
GetStopRequest::process(const TransportManager& tm) const
{
  const auto bus_list = tm.get_stop_schedule(stop_);

  ostringstream ss;
  ss << "Stop " << stop_ << ":";
  if (bus_list) {
    if (bus_list->empty()) {
      ss << " no buses";
    } else {
      ss << " buses";
      for (const auto& bus : *bus_list) {
        ss << " " << bus;
      }
    }
  } else {
    ss << " not found";
  }
  return ss.str();
}

RequestPtr
create_request(Request::Type type)
{
  switch (type) {
    case Request::Type::ADD_BUS:
      return make_unique<AddBusRequest>();
    case Request::Type::ADD_STOP:
      return make_unique<AddBusStopRequest>();
    case Request::Type::GET_BUS:
      return make_unique<GetBusRequest>();
    case Request::Type::GET_STOP:
      return make_unique<GetStopRequest>();
    default:
      break;
  }
  return nullptr;
}

optional<Request::Type>
get_request_type(string_view operation, string_view operand)
{
  static map<string_view, Request::Type> read_requests = {
    { "Bus", Request::Type::GET_BUS }, { "Stop", Request::Type::GET_STOP }
  };
  static map<string_view, Request::Type> write_requests = {
    { "Bus", Request::Type::ADD_BUS }, { "Stop", Request::Type::ADD_STOP }
  };

  bool is_read = [&operand]() {
    auto [arg1, arg2] = SplitTwoStrict(operand, ":");
    return !arg2.has_value();
  }();

  auto& requests = is_read ? read_requests : write_requests;
  if (auto it = requests.find(operation); it != end(requests)) {
    return it->second;
  } else {
    return nullopt;
  }
}

vector<RequestPtr>
read_requests_impl(istream& is = cin)
{
  string number_requests_str;
  getline(is, number_requests_str);
  const auto number_requests = stoi(number_requests_str);

  vector<RequestPtr> requests;
  requests.reserve(number_requests);

  for (auto i = decltype(number_requests){ 0 }; i < number_requests; ++i) {
    string request_str;
    getline(is, request_str);
    if (auto request = read_request(request_str)) {
      requests.push_back(move(request));
    }
  }
  return requests;
}

vector<RequestPtr>
read_requests(istream& is)
{
  auto write_requests = read_requests_impl(is);
  auto read_requests = read_requests_impl(is);

  vector<RequestPtr> requests;
  requests.insert(end(requests),
                  make_move_iterator(begin(write_requests)),
                  make_move_iterator(end(write_requests)));
  requests.insert(end(requests),
                  make_move_iterator(begin(read_requests)),
                  make_move_iterator(end(read_requests)));
  return requests;
}

RequestPtr
read_request(string_view request_str)
{
  const auto operation = ReadToken(request_str);
  const auto& operand = request_str;

  const auto type = get_request_type(operation, operand);
  if (!type) {
    return nullptr;
  }
  auto request = create_request(*type);
  if (request) {
    request->read(operand);
  }
  return request;
}

vector<string>
process_requests(const vector<RequestPtr>& requests)
{
  vector<string> res;

  TransportManager tm;
  for (const auto& request : requests) {
    if (request->type_ == Request::Type::GET_BUS ||
        request->type_ == Request::Type::GET_STOP) {
      const auto& get_bus_request =
        static_cast<const ReadRequest<string>&>(*request);
      res.push_back(get_bus_request.process(tm));
    } else {
      const auto& write_request = static_cast<const WriteRequest&>(*request);
      write_request.process(tm);
    }
  }

  return res;
}

void
print_responses(const vector<string>& responses, ostream& os)
{
  for (const auto& response : responses) {
    os << response << "\n";
  }
}
