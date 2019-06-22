#include "test_runner.h"

#include "transport_manager.h"
#include "utils.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

struct Request
{
  enum class Type
  {
    ADD_BUS,
    ADD_STOP,
    GET_BUS,
    GET_STOP
  };

  Request(Type type)
    : type_{ type }
  {}

  virtual ~Request() = default;
  virtual void read(string_view data) = 0;

  const Type type_;
};

template<typename T>
struct ReadRequest : public Request
{
  using Request::Request;
  virtual T process(const TransportManager& tm) const = 0;
};

struct WriteRequest : public Request
{
  using Request::Request;
  void read(string_view data) override final
  {
    auto [operation, operand] = SplitTwoStrict(data, ": ");
    read(operation, *operand);
  }

  virtual void read(string_view operation, string_view operand) = 0;
  virtual void process(TransportManager& tm) const = 0;
};

struct AddBusRequest : public WriteRequest
{
  AddBusRequest()
    : WriteRequest(Type::ADD_BUS)
  {}

  bool is_one_way(string_view route_desc) const
  {
    return route_desc.find(">") != route_desc.npos;
  }

  void read(string_view operation, string_view operand) override
  {
    stops_data_ = operand;

    bus_ = operation;
    if (is_one_way(stops_data_)) {
      const auto stops_raw = Split(stops_data_, " > ");
      stops_.insert(begin(stops_), begin(stops_raw), end(stops_raw));
    } else {
      const auto stops_raw = Split(stops_data_, " - ");
      if (!stops_raw.empty()) {
        stops_.insert(begin(stops_), begin(stops_raw), end(stops_raw));
        stops_.insert(end(stops_), next(rbegin(stops_raw)), rend(stops_raw));
      }
    }
  }

  void process(TransportManager& tm) const override
  {
    vector<string> stops(begin(stops_), end(stops_));
    tm.add_bus_route(bus_, stops);
  }

  TransportManager::BusId bus_;
  vector<string_view> stops_;
  string stops_data_;
};

struct AddBusStopRequest : public WriteRequest
{
  AddBusStopRequest()
    : WriteRequest(Type::ADD_STOP)
  {}

  void read(string_view operation, string_view operand) override
  {
    stop_name_ = ReadToken(operation, ": ");
    latitude_ = ConvertFromView<double>(ReadToken(operand, ", "));
    longitude_ = ConvertFromView<double>(operand);
  }

  void process(TransportManager& tm) const override
  {
    tm.add_stop(stop_name_, latitude_, longitude_);
  }

  string stop_name_;
  double latitude_ = 0.;
  double longitude_ = 0.;
};

struct GetBusRequest : public ReadRequest<string>
{
  GetBusRequest()
    : ReadRequest<string>(Type::GET_BUS)
  {}

  void read(string_view data) override { bus_ = data; }

  string process(const TransportManager& tm) const override
  {
    const auto total_num = tm.get_total_stop_num(bus_);
    const auto unique_num = tm.get_unique_stops_num(bus_);
    const auto route_length = tm.get_route_length(bus_);

    ostringstream ss;
    ss << "Bus " << bus_ << ": ";
    if (total_num && unique_num && route_length) {
      ss << *total_num << " stops on route, " << *unique_num
         << " unique stops, " << *route_length << " route length";
    } else {
      ss << "not found";
    }
    return ss.str();
  }

  TransportManager::BusId bus_;
};

struct GetStopRequest : public ReadRequest<string>
{
  GetStopRequest()
    : ReadRequest<string>(Type::GET_STOP)
  {}

  void read(string_view data) override { stop_ = data; }

  string process(const TransportManager& tm) const override
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

  TransportManager::StopId stop_;
};

using RequestPtr = unique_ptr<Request>;

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
read_requests(istream& is = cin)
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
print_responses(const vector<string>& responses, ostream& os = cout)
{
  for (const auto& response : responses) {
    os << response << "\n";
  }
}

#ifdef LOCAL_TEST

void
test_total_stops()
{
  TransportManager manager;
  manager.add_stop("yandex", 0., 0.);
  manager.add_stop("google", 0., 0.);
  manager.add_stop("binq", 0., 0.);

  manager.add_bus_route("1", { "yandex", "google", "yandex", "google" });
  manager.add_bus_route("2",
                        { "google", "yandex", "binq", "google", "yandex" });

  ASSERT_EQUAL(*manager.get_total_stop_num("1"), 4);
  ASSERT_EQUAL(*manager.get_total_stop_num("2"), 5);
}

void
test_unique_stops()
{
  TransportManager manager;
  manager.add_stop("yandex", 0., 0.);
  manager.add_stop("google", 0., 0.);
  manager.add_stop("binq", 0., 0.);

  manager.add_bus_route("1", { "yandex", "google", "yandex", "google" });
  manager.add_bus_route("2",
                        { "google", "yandex", "binq", "google", "yandex" });

  ASSERT_EQUAL(*manager.get_unique_stops_num("1"), 2);
  ASSERT_EQUAL(*manager.get_unique_stops_num("2"), 3);
}

void
test_query_order()
{
  TransportManager manager;
  manager.add_stop("yandex", 0., 0.);
  manager.add_bus_route("1", { "yandex", "google", "yandex", "google" });
  manager.add_stop("google", 0., 0.);
  manager.add_bus_route("2",
                        { "google", "yandex", "binq", "google", "yandex" });
  manager.add_stop("binq", 0., 0.);

  ASSERT_EQUAL(*manager.get_unique_stops_num("1"), 2);
  ASSERT_EQUAL(*manager.get_unique_stops_num("2"), 3);
}

void
test_distances()
{
  TransportManager tm;
  tm.add_stop("Tolstopaltsevo", 55.611087, 37.20829);
  tm.add_stop("Marushkino", 55.595884, 37.209755);

  tm.add_bus_route("256",
                   { "Biryulyovo Zapadnoye",
                     "Biryusinka",
                     "Universam",
                     "Biryulyovo Tovarnaya",
                     "Biryulyovo Passazhirskaya",
                     "Biryulyovo Zapadnoye" });
  tm.add_bus_route("750", { "Tolstopaltsevo", "Marushkino", "Rasskazovka" });

  tm.add_stop("Rasskazovka", 55.632761, 37.333324);
  tm.add_stop("Biryulyovo Zapadnoye", 55.574371, 37.6517);
  tm.add_stop("Biryusinka", 55.581065, 37.64839);
  tm.add_stop("Universam", 55.587655, 37.645687);
  tm.add_stop("Biryulyovo Tovarnaya", 55.592028, 37.653656);
  tm.add_stop("Biryulyovo Passazhirskaya", 55.580999, 37.659164);

  const auto length_256 = tm.get_route_length("256");
  const auto length_750 = tm.get_route_length("750");

  ASSERT_EQUAL(fabs(*length_256 - 4371.017250) < 1e-4, true);
  ASSERT_EQUAL(fabs(*length_750 - 10469.741523) < 1e-4, true);
}

void
test_get_stops()
{
  TransportManager manager;
  manager.add_stop("yandex", 0., 0.);
  manager.add_stop("google", 0., 0.);
  manager.add_stop("binq", 0., 0.);

  manager.add_bus_route("1", { "yandex", "google", "yandex", "google" });
  manager.add_bus_route("2",
                        { "google", "yandex", "binq", "google", "yandex" });

  vector<string> bus_list_yandex{ "1", "2" };
  vector<string> bus_list_google{ "1", "2" };
  vector<string> bus_list_binq{ "2" };

  ASSERT_EQUAL(*manager.get_stop_schedule("google"), bus_list_yandex);
  ASSERT_EQUAL(*manager.get_stop_schedule("yandex"), bus_list_google);
  ASSERT_EQUAL(*manager.get_stop_schedule("binq"), bus_list_binq);
}

void
test_readadd_stop()
{
  string request_str("Stop new_stop: 55.5, -90");
  auto request = read_request(request_str);
  auto add_stop_request = dynamic_cast<AddBusStopRequest*>(request.get());
  ASSERT_EQUAL(!!add_stop_request, true);
  ASSERT_EQUAL(add_stop_request->stop_name_, string("new_stop"));
  ASSERT_EQUAL(to_string(add_stop_request->latitude_), to_string(55.5));
  ASSERT_EQUAL(to_string(add_stop_request->longitude_), to_string(-90.));
}

void
test_readadd_bus_one_way()
{
  vector<string_view> ref_stops{ "first", "second", "third" };

  string request_str("Bus 55: first > second > third");
  auto request = read_request(request_str);
  auto add_bus_request = dynamic_cast<AddBusRequest*>(request.get());
  ASSERT_EQUAL(!!add_bus_request, true);
  ASSERT_EQUAL(add_bus_request->bus_, "55");
  ASSERT_EQUAL(add_bus_request->stops_, ref_stops);
}

void
test_readadd_bus_both_ways()
{
  vector<string_view> ref_stops{
    "first", "second", "third", "second", "first"
  };

  string request_str("Bus 55: first - second - third");
  auto request = read_request(request_str);
  auto add_bus_request = dynamic_cast<AddBusRequest*>(request.get());
  ASSERT_EQUAL(!!add_bus_request, true);
  ASSERT_EQUAL(add_bus_request->bus_, "55");
  ASSERT_EQUAL(add_bus_request->stops_, ref_stops);
}

void
test_readadd_bus_extremes()
{
  {
    vector<string_view> ref1{ "first" };
    string request_str("Bus 55: first");
    auto request = read_request(request_str);
    auto add_bus_request = dynamic_cast<AddBusRequest*>(request.get());
    ASSERT_EQUAL(!!add_bus_request, true);
    ASSERT_EQUAL(add_bus_request->bus_, "55");
    ASSERT_EQUAL(add_bus_request->stops_, ref1);
  }
  {
    vector<string_view> ref2{};
    string request_str("Bus 55: ");
    auto request = read_request(request_str);
    auto add_bus_request = dynamic_cast<AddBusRequest*>(request.get());
    ASSERT_EQUAL(!!add_bus_request, true);
    ASSERT_EQUAL(add_bus_request->stops_, ref2);
  }
}

void
test_readget_bus()
{
  string request_str("Bus 34");
  auto request = read_request(request_str);
  auto get_bus_request = dynamic_cast<GetBusRequest*>(request.get());
  ASSERT_EQUAL(!!get_bus_request, true);
  ASSERT_EQUAL(get_bus_request->bus_, "34");
}

void
test_readget_stop()
{
  string request_str("Stop Hello Here");
  auto request = read_request(request_str);
  auto get_stop_request = dynamic_cast<GetStopRequest*>(request.get());
  ASSERT_EQUAL(!!get_stop_request, true);
  ASSERT_EQUAL(get_stop_request->stop_, "Hello Here");
}

void
test_pipeline()
{
  const string input = R"(11
Stop Tolstopaltsevo: 55.611087, 37.20829
Stop Marushkino: 55.595884, 37.209755
Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye
Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka
Stop Rasskazovka: 55.632761, 37.333324
Stop Biryulyovo Zapadnoye: 55.574371, 37.6517
Stop Biryusinka: 55.581065, 37.64839
Stop Universam: 55.587655, 37.645687
Stop Biryulyovo Tovarnaya: 55.592028, 37.653656
Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164
Stop New Stop: 30.0, 40.0
6
Stop Biryulyovo Zapadnoye
Stop New Stop
Bus 256
Stop Google
Bus 750
Bus 751)";

  const string output =
    R"(Stop Biryulyovo Zapadnoye: buses 256
Stop New Stop: no buses
Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length
Stop Google: not found
Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length
Bus 751: not found
)";

  istringstream is(input);
  const auto requests = read_requests(is);
  const auto responses = process_requests(requests);
  ostringstream os;
  print_responses(responses, os);

  ASSERT_EQUAL(string(os.str()), output);
}

#endif // LOCAL_TEST

int
main()
{
#ifdef LOCAL_TEST
  TestRunner tr;
  RUN_TEST(tr, test_total_stops);
  RUN_TEST(tr, test_unique_stops);
  RUN_TEST(tr, test_query_order);
  RUN_TEST(tr, test_distances);
  RUN_TEST(tr, test_get_stops);
  RUN_TEST(tr, test_readadd_stop);
  RUN_TEST(tr, test_readadd_bus_one_way);
  RUN_TEST(tr, test_readadd_bus_both_ways);
  RUN_TEST(tr, test_readadd_bus_extremes);
  RUN_TEST(tr, test_readget_bus);
  RUN_TEST(tr, test_readget_stop);
  RUN_TEST(tr, test_pipeline);

#endif // LOCAL_TEST

  cout << setprecision(6);

  const auto requests = read_requests();
  const auto responses = process_requests(requests);
  print_responses(responses);

  return 0;
}
