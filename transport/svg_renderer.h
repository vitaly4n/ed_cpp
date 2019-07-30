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
  using ToSvgPoint = std::function<Point(double latitude, double longitude)>;

  void RenderLayer(std::string_view layer, Document& doc, ToSvgPoint point_transform) const;
  void RenderRouteLines(Document& doc, ToSvgPoint point_transform) const;
  void RenderBusLabels(Document& doc, ToSvgPoint point_transform) const;
  void RenderStopSigns(Document& doc, ToSvgPoint point_transform) const;
  void RenderStopLabels(Document& doc, ToSvgPoint point_transform) const;

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
