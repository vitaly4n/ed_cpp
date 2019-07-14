#include "tests.h"

#ifdef LOCAL_TEST

#include "json.h"
#include "requests.h"
#include "test_utils.h"
#include "transport_catalog.h"

#include <algorithm>
#include <cmath>
#include <fstream>

using namespace std;

constexpr auto TEST_DIR = STRINGIFY2(TESTING_DIR_transport_E_sanitize);

void
test_json(const string& input_file, const string& output_file)
{
  TestDataHandler test_data(TEST_DIR, input_file.c_str(), output_file.c_str());

  const auto ref_doc = Json::Load(test_data.ReferenceData());
  const auto input_doc = Json::Load(test_data.InputData());
  const auto& input_map = input_doc.GetRoot().AsMap();

  const TransportCatalog db(
    Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
    input_map.at("routing_settings").AsMap());

  Json::Node res =
    Requests::ProcessAll(db, input_map.at("stat_requests").AsArray());

  ostringstream ref_os;
  ostringstream res_os;

  Json::PrintNode(res, res_os);
  Json::PrintNode(ref_doc.GetRoot(), ref_os);

  ASSERT_EQUAL(res_os.str(), ref_os.str());
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

  RUN_TEST(tr, test_json_pipeline_1);
  RUN_TEST(tr, test_json_routes_1);
  RUN_TEST(tr, test_json_routes_2);
  RUN_TEST(tr, test_json_routes_3);
  RUN_TEST(tr, test_json_routes_4);
}

#endif
