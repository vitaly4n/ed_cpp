#include "svg_renderer.h"

#include <algorithm>

using namespace std;

namespace {

struct Bounds
{
  void Add(double value)
  {
    min = std::min(min, value);
    max = std::max(max, value);
  }

  bool IsZero() const { return fabs(max - min) < 1e-6; }
  double GetDiff() const { return max - min; }

  double min = numeric_limits<double>::max();
  double max = numeric_limits<double>::lowest();
};

class TransformToSvg
{
public:
  Svg::Point operator()(double latitude, double longitude) const
  {
    return { (longitude - longitude_bounds_.min) * zoom_coeff_ + padding_,
             (latitude_bounds_.max - latitude) * zoom_coeff_ + padding_ };
  }

private:
  friend class TransformToSvgBuilder;
  TransformToSvg(Bounds latitude_bounds, Bounds longitude_bounds, double zoom_coeff, double padding)
    : latitude_bounds_(latitude_bounds)
    , longitude_bounds_(longitude_bounds)
    , zoom_coeff_(zoom_coeff)
    , padding_(padding)
  {}

  Bounds latitude_bounds_;
  Bounds longitude_bounds_;
  double zoom_coeff_;
  double padding_;
};

class TransformToSvgBuilder
{
public:
  template<typename It, typename GetShpereCoord>
  TransformToSvgBuilder& SetBounds(It first, It last, GetShpereCoord getter)
  {
    for (; first != last; ++first) {
      const Sphere::Point sphere_pt = getter(first);
      latitude_boudns_.Add(sphere_pt.latitude);
      longitude_bounds_.Add(sphere_pt.longitude);
    }
    return *this;
  }

  TransformToSvgBuilder& SetHeight(double height)
  {
    height_ = height;
    return *this;
  }
  TransformToSvgBuilder& SetWidth(double width)
  {
    width_ = width;
    return *this;
  }
  TransformToSvgBuilder& SetPaddint(double padding)
  {
    padding_ = padding;
    return *this;
  }

  operator TransformToSvg() const
  {
    double zoom_coeff = 0.;

    double zoom_coeff_w = numeric_limits<double>::max();
    double zoom_coeff_h = numeric_limits<double>::max();
    if (!latitude_boudns_.IsZero()) {
      zoom_coeff_h = (height_ - 2 * padding_) / latitude_boudns_.GetDiff();
    }
    if (!longitude_bounds_.IsZero()) {
      zoom_coeff_w = (width_ - 2 * padding_) / longitude_bounds_.GetDiff();
    }

    if (!latitude_boudns_.IsZero() || !longitude_bounds_.IsZero()) {
      zoom_coeff = min(zoom_coeff_h, zoom_coeff_w);
    }

    return TransformToSvg(latitude_boudns_, longitude_bounds_, zoom_coeff, padding_);
  }

private:
  Bounds latitude_boudns_;
  Bounds longitude_bounds_;

  double height_ = 0.;
  double width_ = 0.;
  double padding_ = 0.;
};

} // namespace

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
  return visit(
    [](auto var) -> Color {
      using var_type = remove_reference_t<remove_cv_t<decltype(var)>>;
      if constexpr (is_same_v<var_type, string>) {
        return move(var);
      } else if constexpr (is_same_v<var_type, vector<Json::Node>>) {
        Rgba rgba_color;
        rgba_color.red = var[0].AsInt();
        rgba_color.green = var[1].AsInt();
        rgba_color.blue = var[2].AsInt();
        if (var.size() > 3) {
          rgba_color.alpha = var[3].AsDouble();
        }
        return rgba_color;
      }
      return {};
    },
    node.GetBase());
}

template<>
vector<Color>
ParseSvgFromJson(const Json::Node& node)
{
  const auto& color_nodes = node.AsArray();

  vector<Color> colors;
  colors.reserve(color_nodes.size());
  for (const auto& color_node : color_nodes) {
    colors.push_back(ParseSvgFromJson<Color>(color_node));
  }
  return colors;
}

template<>
vector<string>
ParseSvgFromJson(const Json::Node& node)
{
  const auto& layer_nodes = node.AsArray();

  vector<string> layers;
  layers.reserve(layer_nodes.size());
  for (const auto& layer_node : layer_nodes) {
    layers.push_back(layer_node.AsString());
  }
  return layers;
}

MapRenderer::MapRenderer(const Json::Dict& settings)
  : settings_(settings)
{}

void
MapRenderer::AddStop(string name, const Descriptions::Stop& stop)
{
  stops_[move(name)] = stop;
}

void
MapRenderer::AddBus(string name, const Descriptions::Bus& bus)
{
  buses_[move(name)] = bus;
}

string
MapRenderer::Render() const
{
  Document doc;

  TransformToSvg to_svg =
    TransformToSvgBuilder()
      .SetWidth(settings_.width)
      .SetHeight(settings_.height)
      .SetPaddint(settings_.padding)
      .SetBounds(begin(stops_), end(stops_), [](const auto& name_and_stop) { return name_and_stop->second.position; });

  for (const auto& layer : settings_.layers) {
    RenderLayer(layer, doc, to_svg);
  }

  ostringstream os;
  doc.Render(os);
  return os.str();
}

void
MapRenderer::RenderLayer(string_view layer, Document& doc, MapRenderer::ToSvgPoint point_transform) const
{
  if (layer == "bus_lines") {
    RenderRouteLines(doc, move(point_transform));
  } else if (layer == "bus_labels") {
    RenderBusLabels(doc, move(point_transform));
  } else if (layer == "stop_points") {
    RenderStopSigns(doc, move(point_transform));
  } else if (layer == "stop_labels") {
    RenderStopLabels(doc, move(point_transform));
  }
}

void
MapRenderer::RenderRouteLines(Document& doc, MapRenderer::ToSvgPoint point_transform) const
{

  unsigned bus_idx = 0;
  for (const auto& [_, bus] : buses_) {
    if (bus.stops.empty()) {
      continue;
    }

    const Color color = settings_.color_palette[bus_idx++ % settings_.color_palette.size()];
    Polyline route = Polyline()
                       .SetStrokeColor(color)
                       .SetStrokeWidth(settings_.line_width)
                       .SetStrokeLineCap("round")
                       .SetStrokeLineJoin("round");

    for (const auto& stop : bus.stops) {
      const Sphere::Point& sphere_pt = stops_.at(stop).position;
      route.AddPoint(point_transform(sphere_pt.latitude, sphere_pt.longitude));
    }

    doc.Add(move(route));
  }
}
void
MapRenderer::RenderBusLabels(Document& doc, MapRenderer::ToSvgPoint point_transform) const
{

  unsigned bus_idx = 0;
  for (const auto& [_, bus] : buses_) {
    if (bus.routing_stops.empty()) {
      continue;
    }

    const string& bus_name = bus.name;
    const Color& color = settings_.color_palette[bus_idx++ % settings_.color_palette.size()];

    const auto add_bus_label = [&](const auto& stop) {
      const Sphere::Point& sphere_pt = stops_.at(stop).position;
      const auto common = Text()
                            .SetPoint(point_transform(sphere_pt.latitude, sphere_pt.longitude))
                            .SetOffset(settings_.bus_label_offset)
                            .SetFontSize(settings_.bus_label_font_size)
                            .SetFontFamily("Verdana")
                            .SetFontWeight("bold")
                            .SetData(bus_name);

      doc.Add(Text(common)
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap("round")
                .SetStrokeLineJoin("round"));

      doc.Add(Text(common).SetFillColor(color));
    };

    const auto& start_stop = *begin(bus.routing_stops);
    const auto& end_stop = *rbegin(bus.routing_stops);
    add_bus_label(start_stop);
    if (start_stop != end_stop) {
      add_bus_label(end_stop);
    }
  }
}
void
MapRenderer::RenderStopSigns(Document& doc, MapRenderer::ToSvgPoint point_transform) const
{

  for (const auto& [_, stop] : stops_) {
    const Point position = point_transform(stop.position.latitude, stop.position.longitude);

    doc.Add(Circle().SetCenter(position).SetRadius(settings_.stop_radius).SetFillColor("white"));
  }
}

void
MapRenderer::RenderStopLabels(Document& doc, MapRenderer::ToSvgPoint point_transform) const
{

  for (const auto& [_, stop] : stops_) {
    const Point position = point_transform(stop.position.latitude, stop.position.longitude);
    const Text common = Text()
                          .SetPoint(position)
                          .SetOffset(settings_.stop_label_offset)
                          .SetFontSize(settings_.stop_label_font_size)
                          .SetFontFamily("Verdana")
                          .SetData(stop.name);

    doc.Add(Text(common)
              .SetFillColor(settings_.underlayer_color)
              .SetStrokeColor(settings_.underlayer_color)
              .SetStrokeWidth(settings_.underlayer_width)
              .SetStrokeLineCap("round")
              .SetStrokeLineJoin("round"));

    doc.Add(Text(common).SetFillColor("black"));
  }
}

MapRenderer::Settings::Settings(const Json::Dict& settings)
{
  auto set_setting = [&settings](string_view name, auto& setting) {
    const auto& node = settings.at(name.data());
    setting = ParseSvgFromJson<decay_t<decltype(setting)>>(node);
  };

  set_setting("width", width);
  set_setting("height", height);
  set_setting("padding", padding);
  set_setting("stop_radius", stop_radius);
  set_setting("line_width", line_width);
  set_setting("underlayer_width", underlayer_width);
  set_setting("stop_label_font_size", stop_label_font_size);
  set_setting("bus_label_font_size", bus_label_font_size);
  set_setting("stop_label_offset", stop_label_offset);
  set_setting("bus_label_offset", bus_label_offset);
  set_setting("underlayer_color", underlayer_color);
  set_setting("color_palette", color_palette);
  set_setting("layers", layers);
}

size_t
MapRenderer::Settings::LayerCount(std::string_view layer) const
{
  return count(begin(layers), end(layers), layer);
}

} // namespace Svg
