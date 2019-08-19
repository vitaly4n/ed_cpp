#include "tests.h"

#ifdef LOCAL_TEST

#include "json.h"
#include "requests.h"
#include "svg_renderer.h"
#include "test_utils.h"
#include "transport_catalog.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>

using namespace std;

constexpr auto TEST_DIR = STRINGIFY2(TESTING_DIR_transport);

void
test_json(const string& input_file, const string& output_file)
{
  TestDataHandler test_data(TEST_DIR, input_file.c_str(), output_file.c_str());

  const auto ref_doc = Json::Load(test_data.ReferenceData());
  const auto input_doc = Json::Load(test_data.InputData());
  const auto& input_map = input_doc.GetRoot().AsMap();

  unique_ptr<MapRenderer> renderer;
  auto render_settings_it = input_map.find("render_settings");
  if (render_settings_it != input_map.end()) {
    renderer = make_unique<Svg::MapRenderer>(render_settings_it->second.AsMap());
  }

  const TransportCatalog db(Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
                            input_map.at("routing_settings").AsMap(),
                            move(renderer));

  Json::Node res = Requests::ProcessAll(db, input_map.at("stat_requests").AsArray());

  ostringstream ref_os;
  ostringstream res_os;

  Json::PrintNode(res, res_os);
  Json::PrintNode(ref_doc.GetRoot(), ref_os);

  ASSERT_EQUAL(res_os.str(), ref_os.str());
}

void
test_svg(const string& input_file_json, const string& output_file_svg)
{
  TestDataHandler test_data(TEST_DIR, input_file_json.c_str(), output_file_svg.c_str());

  auto& ref_stream = test_data.ReferenceData();
  const auto input_doc = Json::Load(test_data.InputData());
  const auto& input_map = input_doc.GetRoot().AsMap();

  unique_ptr<MapRenderer> renderer;
  auto render_settings_it = input_map.find("render_settings");
  if (render_settings_it != input_map.end()) {
    renderer = make_unique<Svg::MapRenderer>(render_settings_it->second.AsMap());
  }

  const TransportCatalog db(Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
                            input_map.at("routing_settings").AsMap(),
                            move(renderer));

  const string res_map = db.RenderMap();
  const string ref_map(istreambuf_iterator<char>(ref_stream), {});

  ofstream is(std::string(TEST_DIR) + "/test_res" + output_file_svg, ofstream::trunc);
  is << res_map;

  ASSERT_EQUAL(res_map, ref_map);
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
test_svg_1()
{
  test_svg("in_svg_1.json", "out_svg_1.svg");
}

void
test_svg_2()
{
  test_svg("in_svg_2.json", "out_svg_2.svg");
}

void
run_tests()
{
  TestRunner tr;

  RUN_TEST(tr, test_json_pipeline_1);
  RUN_TEST(tr, test_json_routes_1);
  RUN_TEST(tr, test_json_routes_2);
  RUN_TEST(tr, test_json_routes_3);
  RUN_TEST(tr, test_json_routes_4);
  RUN_TEST(tr, test_svg_1);
  RUN_TEST(tr, test_svg_2);
}

#endif
