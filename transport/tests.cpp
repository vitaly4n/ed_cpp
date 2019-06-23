#include "tests.h"

#include "request_plain.h"
#include "transport_manager.h"

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
                     "Biryulyovo Zapadnoye" });
  tm.add_bus_route("750", { "Tolstopaltsevo", "Marushkino", "Rasskazovka" });

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

  tm.add_bus_route("750", { "Tolstopaltsevo", "Marushkino", "Rasskazovka" });

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
}

#endif
