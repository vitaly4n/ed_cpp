#include "application.h"

#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "svg.h"
#include "svg_renderer.h"
#include "transport_catalog.h"

using namespace std;

void
RunBase(istream& is)
{
  const auto input_doc = Json::Load(is);
  const auto& input_map = input_doc.GetRoot().AsMap();

  const auto& base_requests = input_map.at("base_requests").AsArray();
  const auto& routing_settings = input_map.at("routing_settings").AsMap();
  const auto& render_settings = input_map.at("render_settings").AsMap();
  const auto& serialization_settings = input_map.at("serialization_settings").AsMap();

  const TransportCatalog db(
    Descriptions::ReadDescriptions(base_requests), routing_settings, make_unique<Svg::MapRenderer>(render_settings));

  db.Serialize(serialization_settings);
}

void
RunProcessRequests(istream& is)
{
  const auto input_doc = Json::Load(is);
  const auto& input_map = input_doc.GetRoot().AsMap();

  const auto& stat_requests = input_map.at("stat_requests").AsArray();
  const auto& serialization_settings = input_map.at("serialization_settings").AsMap();

  const TransportCatalog db = TransportCatalog::Deserialize(serialization_settings);
  Json::PrintValue(Requests::ProcessAll(db, stat_requests), cout);
  cout << endl;
}
