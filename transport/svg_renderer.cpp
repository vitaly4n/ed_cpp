#include "svg_renderer.h"

using namespace std;

namespace Svg {

template<typename T>
T
ParseSvgFromJson(const Json::Node&)
{
  return T{};
}

template<>
double
ParseSvgFromJson(const Json::Node& node)
{
  return node.AsDouble();
}

template<>
int
ParseSvgFromJson(const Json::Node& node)
{
  return node.AsInt();
}

template<>
Point
ParseSvgFromJson(const Json::Node& node)
{
  const auto& data = node.AsArray();
  return { data.at(0).AsDouble(), data.at(1).AsDouble() };
}

template<>
Color
ParseSvgFromJson(const Json::Node& node)
{
  // TODO
  return Color{};
}

template<>
std::vector<Color>
ParseSvgFromJson(const Json::Node& node)
{
  // TODO
  return {};
}

MapRenderer::MapRenderer(const Json::Dict& settings)
  : settings_(settings)
{}

void
MapRenderer::AddStop(std::string name, const Responses::Stop& stop)
{
  // TODO
}

void
MapRenderer::AddBus(std::string name, const Responses::Bus& bus)
{
  // TODO
}

std::string
MapRenderer::Render() const
{
  return {};
}

MapRenderer::Settings::Settings(const Json::Dict& settings)
{
  auto set_setting = [&settings](string_view name, auto& setting) {
    const auto& node = settings.at(name.data());
    setting = ParseSvgFromJson<std::decay_t<decltype(setting)>>(node);
  };

  set_setting("width", width);
  set_setting("height", height);
  set_setting("padding", padding);
  set_setting("stop_radius", stop_radius);
  set_setting("line_width", line_width);
  set_setting("underlayer_width", underlayer_width);
  set_setting("stop_label_font_size", stop_label_font_size);
  set_setting("stop_label_offset", stop_label_offset);
  set_setting("underlayer_color", underlayer_color);
  set_setting("color_palette", color_palette);
}

} // namespace Svg
