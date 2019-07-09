#include "tests.h"

#include "request_json.h"
#include "request_plain.h"
#include "transport_manager.h"

#include <fstream>

bool
AreNodesEqual(const Json::Node& lhs, const Json::Node& rhs);

bool
AreMapNodesEqual(const Json::Node& lhs, const Json::Node& rhs)
{
  const auto& lhs_obj = lhs.AsMap();
  const auto& rhs_obj = rhs.AsMap();

  if (lhs_obj.size() != rhs_obj.size()) {
    return false;
  }

  for (const auto& [lhs_key, lhs_val] : lhs_obj) {
    auto rhs_it = rhs_obj.find(lhs_key);
    if (rhs_it == end(rhs_obj) || !AreNodesEqual(rhs_it->second, lhs_val)) {
      return false;
    }
  }
  return true;
}

bool
AreArrayNodesEqual(const Json::Node& lhs, const Json::Node& rhs)
{
  const auto& lhs_arr = lhs.AsArray();
  const auto& rhs_arr = rhs.AsArray();

  if (lhs_arr.size() != rhs_arr.size()) {
    return false;
  }

  for (const auto& lhs_val : lhs_arr) {
    if (none_of(begin(rhs_arr), end(rhs_arr), [&lhs_val](const auto& rhs_val) {
          return AreNodesEqual(lhs_val, rhs_val);
        })) {
      return false;
    }
  }
  return true;
}

bool
AreDoubleNodesEqual(const Json::Node& lhs, const Json::Node& rhs)
{
  return fabs(lhs.AsDouble() - rhs.AsDouble()) < 1e-4;
}

bool
AreIntNodesEqual(const Json::Node& lhs, const Json::Node& rhs)
{
  return lhs.AsInt() == rhs.AsInt();
}

bool
AreBoolNodesEqual(const Json::Node& lhs, const Json::Node& rhs)
{
  return lhs.AsBool() == rhs.AsBool();
}

bool
AreStringNodesEquL(const Json::Node& lhs, const Json::Node& rhs)
{
  return lhs.AsString() == rhs.AsString();
}

bool
AreNodesEqual(const Json::Node& lhs, const Json::Node& rhs)
{
  using Type = Json::Node::Type;

  const auto lhs_type = lhs.GetType();
  const auto rhs_type = rhs.GetType();

  bool res = false;
  switch (lhs_type) {
    case Type::eArray:
      if (rhs_type == Type::eArray) {
        res = AreArrayNodesEqual(lhs, rhs);
      }
      break;
    case Type::eMap:
      if (rhs_type == Type::eMap) {
        res = AreMapNodesEqual(lhs, rhs);
      }
      break;
    case Type::eString:
      if (rhs_type == Type::eString) {
        res = AreStringNodesEquL(lhs, rhs);
      }
      break;
    case Type::eDouble:
      if (rhs_type == Type::eDouble || rhs_type == Type::eInt) {
        res = AreDoubleNodesEqual(lhs, rhs);
      }
      break;
    case Type::eInt:
      if (rhs_type == Type::eInt) {
        res = AreIntNodesEqual(lhs, rhs);
      } else if (rhs_type == Type::eDouble) {
        res = AreDoubleNodesEqual(lhs, rhs);
      }
      break;
    case Type::eBool:
      if (rhs_type == Type::eBool) {
        res = AreBoolNodesEqual(lhs, rhs);
      }
      break;
    default:
      break;
  }
  return res;
}

#ifdef LOCAL_TEST

constexpr auto TEST_DIR = STRINGIFY_2(TESTING_DIR_transport);

void
test_total_stops()
{
  TransportManager manager;
  manager.add_stop("yandex", 0., 0.);
  manager.add_stop("google", 0., 0.);
  manager.add_stop("binq", 0., 0.);

  manager.add_bus_route("1", { "yandex", "google", "yandex", "google" }, true);
  manager.add_bus_route(
    "2", { "google", "yandex", "binq", "google", "yandex" }, true);

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

  manager.add_bus_route("1", { "yandex", "google", "yandex", "google" }, true);
  manager.add_bus_route(
    "2", { "google", "yandex", "binq", "google", "yandex" }, true);

  ASSERT_EQUAL(*manager.get_unique_stops_num("1"), 2);
  ASSERT_EQUAL(*manager.get_unique_stops_num("2"), 3);
}

void
test_query_order()
{
  TransportManager manager;
  manager.add_stop("yandex", 0., 0.);
  manager.add_bus_route("1", { "yandex", "google", "yandex", "google" }, true);
  manager.add_stop("google", 0., 0.);
  manager.add_bus_route(
    "2", { "google", "yandex", "binq", "google", "yandex" }, true);
  manager.add_stop("binq", 0., 0.);

  ASSERT_EQUAL(*manager.get_unique_stops_num("1"), 2);
  ASSERT_EQUAL(*manager.get_unique_stops_num("2"), 3);
}

void
test_distances_geo()
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
                     "Biryulyovo Zapadnoye" },
                   true);
  tm.add_bus_route(
    "750", { "Tolstopaltsevo", "Marushkino", "Rasskazovka" }, true);

  tm.add_stop("Rasskazovka", 55.632761, 37.333324);
  tm.add_stop("Biryulyovo Zapadnoye", 55.574371, 37.6517);
  tm.add_stop("Biryusinka", 55.581065, 37.64839);
  tm.add_stop("Universam", 55.587655, 37.645687);
  tm.add_stop("Biryulyovo Tovarnaya", 55.592028, 37.653656);
  tm.add_stop("Biryulyovo Passazhirskaya", 55.580999, 37.659164);

  const auto length_256 =
    tm.get_route_length("256", TransportManager::DistanceType::GEO);
  const auto length_750 =
    tm.get_route_length("750", TransportManager::DistanceType::GEO);

  ASSERT_EQUAL(fabs(*length_256 - 4371.017250) < 1e-4, true);
  ASSERT_EQUAL(fabs(*length_750 - 10469.741523) < 1e-4, true);
}

void
test_distances_roads()
{
  TransportManager tm;
  tm.add_stop(
    "Tolstopaltsevo", 55.611087, 37.20829, { { "Marushkino", 6000. } });
  tm.add_stop("Marushkino", 55.595884, 37.209755);

  tm.add_bus_route(
    "750", { "Tolstopaltsevo", "Marushkino", "Rasskazovka" }, true);

  tm.add_stop("Rasskazovka", 55.632761, 37.333324, { { "Marushkino", 5000. } });

  const auto length_750 =
    tm.get_route_length("750", TransportManager::DistanceType::ROADS);

  ASSERT_EQUAL(fabs(*length_750 - 11000.) < 1e-4, true);
}

void
test_get_stops()
{
  TransportManager manager;
  manager.add_stop("yandex", 0., 0.);
  manager.add_stop("google", 0., 0.);
  manager.add_stop("binq", 0., 0.);

  manager.add_bus_route("1", { "yandex", "google", "yandex", "google" }, true);
  manager.add_bus_route(
    "2", { "google", "yandex", "binq", "google", "yandex" }, true);

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
  vector<string_view> ref_stops{ "first", "second", "third" };

  string request_str("Bus 55: first - second - third");
  auto request = read_request(request_str);
  auto add_bus_request = dynamic_cast<AddBusRequest*>(request.get());
  ASSERT_EQUAL(!!add_bus_request, true);
  ASSERT_EQUAL(add_bus_request->bus_, "55");
  ASSERT_EQUAL(add_bus_request->stops_, ref_stops);
  ASSERT_EQUAL(add_bus_request->is_roundtrip_, false);
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
test_pipeline(const string& input, const string& output)
{
  istringstream is(input);
  const auto requests = read_requests(is);
  const auto responses = process_requests(requests);
  ostringstream os;
  print_responses(responses, os);

  ASSERT_EQUAL(string(os.str()), output);
}

void
test_pipeline_v3()
{
  const string input = R"(13
Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino
Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka
Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye
Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka
Stop Rasskazovka: 55.632761, 37.333324
Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam
Stop Biryusinka: 55.581065, 37.64839, 750m to Universam
Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya
Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya
Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye
Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye
Stop Rossoshanskaya ulitsa: 55.595579, 37.605757
Stop Prazhskaya: 55.611678, 37.603831
6
Bus 256
Bus 750
Bus 751
Stop Samara
Stop Prazhskaya
Stop Biryulyovo Zapadnoye)";

  const string output =
    R"(Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.36124 curvature
Bus 750: 5 stops on route, 3 unique stops, 27600 route length, 1.31808 curvature
Bus 751: not found
Stop Samara: not found
Stop Prazhskaya: no buses
Stop Biryulyovo Zapadnoye: buses 256 828
)";

  test_pipeline(input, output);
}

void
test_json_add_bus()
{
  istringstream input(R"({
      "type": "Bus",
      "name": "256",
      "stops": [
        "Biryulyovo Zapadnoye",
        "Biryusinka",
        "Universam",
        "Biryulyovo Tovarnaya",
        "Biryulyovo Passazhirskaya",
        "Biryulyovo Zapadnoye"
      ],
      "is_roundtrip": true
    })");

  auto doc = Json::Load(input);
  auto request = Json::ParseModifyRequest(doc.GetRoot());
  auto* add_bus_request = dynamic_cast<Json::AddBusRequest*>(request.get());
  ASSERT_EQUAL(!!add_bus_request, true);
  ASSERT_EQUAL(add_bus_request->bus(), string("256"));
  ASSERT_EQUAL(add_bus_request->stops(),
               vector<string>({ "Biryulyovo Zapadnoye",
                                "Biryusinka",
                                "Universam",
                                "Biryulyovo Tovarnaya",
                                "Biryulyovo Passazhirskaya",
                                "Biryulyovo Zapadnoye" }));
}

void
test_json_add_stop()
{
  istringstream input(R"({
      "type": "Stop",
      "road_distances": {
        "Rasskazovka": 9900
      },
      "longitude": 37.209755,
      "name": "Marushkino",
      "latitude": 55.595884
    })");

  auto doc = Json::Load(input);
  auto request = Json::ParseModifyRequest(doc.GetRoot());
  auto* add_stop_request = dynamic_cast<Json::AddStopRequest*>(request.get());
  ASSERT_EQUAL(!!add_stop_request, true);
  ASSERT_EQUAL(to_string(add_stop_request->latitude()), to_string(55.595884));
  ASSERT_EQUAL(to_string(add_stop_request->longitude()), to_string(37.209755));
  ASSERT_EQUAL(add_stop_request->stop(), string("Marushkino"));

  auto res = add_stop_request->record();
  decltype(res) ref{ { "Rasskazovka", 9900 } };
  ASSERT_EQUAL(add_stop_request->record().size(), ref.size());
  for (auto i = 0; i < ref.size(); ++i) {
    ASSERT_EQUAL(res[i].first, ref[i].first);
    ASSERT_EQUAL(to_string(res[i].second), to_string(ref[i].second));
  }
}

void
test_json_set_settings()
{
  istringstream input(R"({
    "routing_settings": {
      "bus_wait_time": 6,
      "bus_velocity": 40
    }
  })");

  auto doc = Json::Load(input);
  auto request = Json::ParseSetSettingsRequest(doc.GetRoot());
  auto* set_settings_request =
    dynamic_cast<Json::SetSettingsRequest*>(request.get());
  ASSERT_EQUAL(!!set_settings_request, true);
  ASSERT_EQUAL(to_string(set_settings_request->BusVelocity()), to_string(40.));
  ASSERT_EQUAL(set_settings_request->BusWaitTime(), 6);
}

void
test_json_get_bus()
{
  istringstream input(R"({
      "type": "Bus",
      "name": "788 81",
      "id": 746888088
    })");

  auto doc = Json::Load(input);
  auto request = Json::ParseReadRequest(doc.GetRoot());
  auto* get_bus_request = dynamic_cast<Json::GetBusRequest*>(request.get());
  ASSERT_EQUAL(!!get_bus_request, true);
  ASSERT_EQUAL(get_bus_request->GetID(), 746888088);
  ASSERT_EQUAL(get_bus_request->bus(), "788 81");
}

void
test_json_get_stop()
{
  istringstream input(R"({
      "type": "Stop",
      "name": "Samara",
      "id": 746888088
    })");

  auto doc = Json::Load(input);
  auto request = Json::ParseReadRequest(doc.GetRoot());
  auto* get_stop_request = dynamic_cast<Json::GetStopRequest*>(request.get());
  ASSERT_EQUAL(!!get_stop_request, true);
  ASSERT_EQUAL(get_stop_request->GetID(), 746888088);
  ASSERT_EQUAL(get_stop_request->stop(), "Samara");
}

void
test_json_get_route()
{
  istringstream input(R"({
    "type": "Route",
    "from": "Biryulyovo Zapadnoye",
    "to": "Universam",
    "id": 4  
  })");

  auto doc = Json::Load(input);
  auto request = Json::ParseReadRequest(doc.GetRoot());
  auto* get_route_request = dynamic_cast<Json::GetRouteRequest*>(request.get());
  ASSERT_EQUAL(!!get_route_request, true);
  ASSERT_EQUAL(get_route_request->From(), string("Biryulyovo Zapadnoye"));
  ASSERT_EQUAL(get_route_request->To(), string("Universam"));
}

void
test_json(const string& input_file, const string& output_file)
{
  const string test_dir(TEST_DIR);
  const auto input_path = test_dir + string("/") + input_file;
  const auto output_path = test_dir + string("/") + output_file;

  ifstream in(input_path);
  ifstream ref(output_path);

  auto in_doc = Json::Load(in);

  Json::RequestsHandler rh;
  rh.Parse(in_doc.GetRoot());

  auto res_doc = Json::Document(rh.Process());
  auto ref_doc = Json::Load(ref);

  if (!AreNodesEqual(res_doc.GetRoot(), ref_doc.GetRoot())) {
    ostringstream res_stream;
    Json::Unload(res_stream, res_doc);
    ostringstream ref_stream;
    Json::Unload(ref_stream, ref_doc);

    ASSERT_EQUAL(string(res_stream.str()), string(ref_stream.str()));
  }
}

void
test_json_pipeline_1()
{
  test_json("in_pipeline.json", "out_pipeline.json");
}

void
test_json_routes_1()
{
  test_json("in_routes_1.json", "out_routes_1.json");
}

void
test_json_routes_2()
{
  test_json("in_routes_2.json", "out_routes_2.json");
}

void
test_json_routes_3()
{
  test_json("in_routes_3.json", "out_routes_3.json");
}

void
test_json_routes_4()
{
  test_json("in_routes_4.json", "out_routes_4.json");
}

void
run_tests()
{
  TestRunner tr;
  RUN_TEST(tr, test_total_stops);
  RUN_TEST(tr, test_unique_stops);
  RUN_TEST(tr, test_query_order);
  RUN_TEST(tr, test_distances_geo);
  RUN_TEST(tr, test_distances_roads);
  RUN_TEST(tr, test_get_stops);

  RUN_TEST(tr, test_readadd_stop);
  RUN_TEST(tr, test_readadd_bus_one_way);
  RUN_TEST(tr, test_readadd_bus_both_ways);
  RUN_TEST(tr, test_readadd_bus_extremes);
  RUN_TEST(tr, test_readget_bus);
  RUN_TEST(tr, test_readget_stop);
  RUN_TEST(tr, test_pipeline_v3);

  RUN_TEST(tr, test_json_add_bus);
  RUN_TEST(tr, test_json_add_stop);
  RUN_TEST(tr, test_json_set_settings);
  RUN_TEST(tr, test_json_get_bus);
  RUN_TEST(tr, test_json_get_stop);
  RUN_TEST(tr, test_json_get_route);
  RUN_TEST(tr, test_json_pipeline_1);
  RUN_TEST(tr, test_json_routes_1);
  RUN_TEST(tr, test_json_routes_2);
  RUN_TEST(tr, test_json_routes_3);
//  RUN_TEST(tr, test_json_routes_4);
}

#endif
