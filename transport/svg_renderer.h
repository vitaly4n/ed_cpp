#pragma once

#include "svg.h"
#include "transport_catalog.h"

namespace Svg {

class MapRenderer : public ::MapRenderer
{
public:
  MapRenderer(const Json::Dict& settings);

  void AddStop(std::string name, const Responses::Stop& stop) override;
  void AddBus(std::string name, const Responses::Bus& bus) override;

  std::string Render() const override;

private:
  struct Settings
  {
    explicit Settings(const Json::Dict& settings);

    double width = 0.;
    double height = 0.;
    double padding = 0.;
    double stop_radius = 0.;
    double line_width = 0.;
    double underlayer_width = 0.;
    int stop_label_font_size = 0.;
    Point stop_label_offset;
    Color underlayer_color;
    std::vector<Color> color_palette;
  } settings_;

};

}
