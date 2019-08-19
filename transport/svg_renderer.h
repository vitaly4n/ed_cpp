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

  void AddStop(std::string name, const Descriptions::Stop& stop) override;
  void AddBus(std::string name, const Descriptions::Bus& bus) override;

  std::string Render() const override;

private:
  using StopPointMap = std::unordered_map<std::string_view, Point>;

  void RenderLayer(std::string_view layer, Document& doc, const StopPointMap& point_map) const;
  void RenderRouteLines(Document& doc, const StopPointMap& point_map) const;
  void RenderBusLabels(Document& doc, const StopPointMap& point_map) const;
  void RenderStopSigns(Document& doc, const StopPointMap& point_map) const;
  void RenderStopLabels(Document& doc, const StopPointMap& point_map) const;

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
};

}
