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

class UniformSvgMapper
{
public:
  UniformSvgMapper& SetStops(const map<string, Descriptions::Stop>& stops, const map<string, Descriptions::Bus>& buses)
  {
    struct BusRouteData
    {
      map<string_view, set<unsigned>> route_data_;
      void Insert(string_view bus, unsigned stop_num) { route_data_[bus].insert(stop_num); }
      bool IsNeighbour(string_view bus, unsigned neighbour_stop_num) const
      {
        auto it = route_data_.find(bus);
        if (it != end(route_data_)) {
          return it->second.count(neighbour_stop_num + 1) || it->second.count(neighbour_stop_num - 1);
        }
        return false;
      }
      bool IsNeighboring(const BusRouteData& other) const {
        for (const auto& [this_bus, this_stop_nums] : route_data_) {
          for (const auto& this_stop_num : this_stop_nums) {
            if (other.IsNeighbour(this_bus, this_stop_num)) {
              return true;
            }
          }
        }
        return false;
      }
    };

    unordered_map<string_view, BusRouteData> stops_schedules;
    for (const auto& [bus_name, bus] : buses) {
      int stop_num = 0;
      for (const auto& stop : bus.stops) {
        BusRouteData& bus_data = stops_schedules[stop];
        bus_data.Insert(bus_name, stop_num);
        ++stop_num;
      }
    }

    auto StopListsCanBeMerged = [&](const set<string_view>& lhs, const set<string_view>& rhs) {
      for (const string_view& lhs_stop : lhs) {
        const BusRouteData& lhs_data = stops_schedules[lhs_stop];
        for (const string_view& rhs_stop : rhs) {
          const BusRouteData& rhs_data = stops_schedules[rhs_stop];
          if (lhs_data.IsNeighboring(rhs_data)) {
            return false;
          }
        }
      }
      return true;
    };

    auto shrink = [&](auto first, auto last) {
      for (auto it = first; it != last;) {
        auto to_merge_it = next(it);
        for (; to_merge_it != last; ++to_merge_it) {
          if (StopListsCanBeMerged(it->second, to_merge_it->second)) {
            it->second.insert(begin(to_merge_it->second), end(to_merge_it->second));
            to_merge_it->second.clear();
          } else {
            break;
          }
        }
        it = to_merge_it;
      }
    };

    map<double, set<string_view>> longitude_sort;
    map<double, set<string_view>> latitude_sort;
    for (const auto& [name, stop] : stops) {
      longitude_sort[stop.position.longitude].insert(name);
      latitude_sort[stop.position.latitude].insert(name);
    }

    shrink(begin(longitude_sort), end(longitude_sort));
    shrink(begin(latitude_sort), end(latitude_sort));

    for (const auto& [_, stop_names] : longitude_sort) {
      bool increment = false;
      for (const auto& stop_name : stop_names) {
        x_uniform_mapping_[stop_name] = x_steps_;
        increment = true;
      }
      if (increment) {
        ++x_steps_;
      }
    }
    for (const auto& [_, stop_names] : latitude_sort) {
      bool increment = false;
      for (const auto& stop_name : stop_names) {
        y_uniform_mapping_[stop_name] = y_steps_;
        increment = true;
      }
      if (increment) {
        ++y_steps_;
      }
    }

    return *this;
  }
  UniformSvgMapper& SetHeight(double height)
  {
    height_ = height;
    return *this;
  }
  UniformSvgMapper& SetWidth(double width)
  {
    width_ = width;
    return *this;
  }
  UniformSvgMapper& SetPaddint(double padding)
  {
    padding_ = padding;
    return *this;
  }

  Svg::Point operator()(string_view stop_name)
  {
    const double x_step = x_steps_ > 1 ? (width_ - 2 * padding_) / (x_steps_ - 1) : 0;
    const double y_step = y_steps_ > 1 ? (height_ - 2 * padding_) / (y_steps_ - 1) : 0;

    const unsigned x_idx = x_uniform_mapping_.at(stop_name);
    const unsigned y_idx = y_uniform_mapping_.at(stop_name);

    return { x_idx * x_step + padding_, height_ - padding_ - y_idx * y_step };
  }

private:
  map<string_view, unsigned> x_uniform_mapping_;
  map<string_view, unsigned> y_uniform_mapping_;

  unsigned x_steps_ = 0;
  unsigned y_steps_ = 0;

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

  UniformSvgMapper unimapper;
  unimapper.SetWidth(settings_.width)
    .SetHeight(settings_.height)
    .SetPaddint(settings_.padding)
    .SetStops(stops_, buses_);

  StopPointMap point_map;
  for (auto& stop : stops_) {
    point_map[stop.first] = unimapper(stop.first);
  }

  for (const auto& layer : settings_.layers) {
    RenderLayer(layer, doc, point_map);
  }

  ostringstream os;
  doc.Render(os);
  return os.str();
}

void
MapRenderer::RenderLayer(string_view layer, Document& doc, const StopPointMap& point_map) const
{
  if (layer == "bus_lines") {
    RenderRouteLines(doc, point_map);
  } else if (layer == "bus_labels") {
    RenderBusLabels(doc, point_map);
  } else if (layer == "stop_points") {
    RenderStopSigns(doc, point_map);
  } else if (layer == "stop_labels") {
    RenderStopLabels(doc, point_map);
  }
}

void
MapRenderer::RenderRouteLines(Document& doc, const StopPointMap& point_map) const
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
      route.AddPoint(point_map.at(stop));
    }

    doc.Add(move(route));
  }
}
void
MapRenderer::RenderBusLabels(Document& doc, const StopPointMap& point_map) const
{

  unsigned bus_idx = 0;
  for (const auto& [_, bus] : buses_) {
    if (bus.routing_stops.empty()) {
      continue;
    }

    const string& bus_name = bus.name;
    const Color& color = settings_.color_palette[bus_idx++ % settings_.color_palette.size()];

    const auto add_bus_label = [&](const auto& stop) {
      const auto common = Text()
                            .SetPoint(point_map.at(stop))
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
MapRenderer::RenderStopSigns(Document& doc, const StopPointMap& point_map) const
{

  for (const auto& [stop_name, stop] : stops_) {
    doc.Add(Circle().SetCenter(point_map.at(stop_name)).SetRadius(settings_.stop_radius).SetFillColor("white"));
  }
}

void
MapRenderer::RenderStopLabels(Document& doc, const StopPointMap& point_map) const
{

  for (const auto& [stop_name, stop] : stops_) {
    const Point position = point_map.at(stop_name);
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
