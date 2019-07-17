#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "sphere.h"
#include "svg_renderer.h"
#include "transport_catalog.h"
#include "utils.h"

#include <iostream>

#ifdef LOCAL_TEST

#include "tests.h"

#endif

using namespace std;

int
main()
{
#ifdef LOCAL_TEST
  run_tests();

#endif

  const auto input_doc = Json::Load(cin);
  const auto& input_map = input_doc.GetRoot().AsMap();

  const TransportCatalog db(Descriptions::ReadDescriptions(input_map.at("base_requests").AsArray()),
                            input_map.at("routing_settings").AsMap(),
                            make_unique<Svg::MapRenderer>(input_map.at("render_settings").AsMap()));

  Json::PrintValue(Requests::ProcessAll(db, input_map.at("stat_requests").AsArray()), cout);
  cout << endl;

  return 0;
}
