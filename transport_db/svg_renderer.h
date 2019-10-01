#pragma once

#include "sphere.h"
#include "svg.h"
#include "transport_catalog.h"

#include <functional>

namespace Svg {

class MapRenderer : public ::MapRenderer
{
public:
  MapRenderer(const Json::Dict& settings);

  void Init(std::map<std::string, Descriptions::Stop> stops, std::map<std::string, Descriptions::Bus> buses) override;
  std::string Render() const override;

  std::string RenderRoute(const TransportRouter::RouteInfo& route_info) const override;

private:
  using StopPointMap = std::unordered_map<std::string_view, Point>;
  using RouteInfo = TransportRouter::RouteInfo;

  void InitMap();
  void InitStopCoords();
  void InitBusColors();

  struct RouteData
  {
    std::string_view bus;
    std::vector<std::string_view> stops;
  };

  using RouteDataArray = std::vector<RouteData>;
  RouteDataArray BuildRouteDataArray(const TransportRouter::RouteInfo& route_info) const;

  void RenderLayer(std::string_view layer);
  void RenderRouteLines();
  void RenderBusLabels();
  void RenderStopSigns();
  void RenderStopLabels();

  void RenderRouteLayer(std::string_view layer, Document& route_doc, const RouteDataArray& route_data_array) const;
  void RenderRouteLines(Document& route_doc, const RouteDataArray& route_data_array) const;
  void RenderRouteLabels(Document& route_doc, const RouteDataArray& route_data_array) const;
  void RenderRouteStopSigns(Document& route_doc, const RouteDataArray& route_data_array) const;
  void RenderRouteStopLabels(Document& route_doc, const RouteDataArray& route_data_array) const;

  void RenderRouteLine(Document& doc, std::string_view bus, const std::vector<std::string_view>& stops) const;
  void RenderRouteLabel(Document& doc, std::string_view bus, std::string_view stop) const;
  void RenderStopSign(Document& doc, std::string_view stop) const;
  void RenderStopLabel(Document& doc, std::string_view stop) const;

  struct Settings
  {
    explicit Settings(const Json::Dict& settings);

    size_t LayerCount(std::string_view layer) const;

    double width = 0.;
    double height = 0.;
    double padding = 0.;
    double stop_radius = 0.;
    double line_width = 0.;
    double underlayer_width = 0.;
    double outer_margin = 0.;
    int stop_label_font_size = 0;
    int bus_label_font_size = 0;
    Point stop_label_offset;
    Point bus_label_offset;
    Color underlayer_color;
    std::vector<Color> color_palette;
    std::vector<std::string> layers;
  } settings_;

  std::map<std::string, Descriptions::Stop> stops_;
  std::map<std::string, Descriptions::Bus> buses_;

  std::map<std::string_view, Color> bus_colors_;
  std::map<std::string_view, Point> stop_coords_;
  Document doc_map_;
};

}
