#include "svg_renderer.h"

#include <algorithm>

using namespace std;

namespace {

struct StopRouteData
{
  map<string_view, set<size_t>> route_data_;
  Sphere::Point stop_position_;
  string_view stop_name_;

  StopRouteData(const Descriptions::Stop& stop, const map<string, Descriptions::Bus>& buses)
    : stop_position_(stop.position)
    , stop_name_(stop.name)
  {
    for (const auto& [bus_name, bus] : buses) {
      size_t stop_num = 0;

      auto bus_it = route_data_.lower_bound(bus_name);
      for (const auto& bus_stop : bus.stops) {
        if (stop_name_ == bus_stop) {
          if (bus_it == end(route_data_) || bus_it->first != bus_name) {
            bus_it = route_data_.emplace_hint(bus_it, bus_name, set<size_t>{});
          }
          bus_it->second.insert(stop_num);
        }
        ++stop_num;
      }
    }
  }

  bool IsNeighbour(string_view bus, size_t neighbour_stop_num) const
  {
    auto it = route_data_.find(bus);
    if (it != end(route_data_)) {
      return it->second.count(neighbour_stop_num + 1) || it->second.count(neighbour_stop_num - 1);
    }
    return false;
  }
  bool IsNeighboring(const StopRouteData& other) const
  {
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

class UniformSvgMapper
{
public:
  UniformSvgMapper& SetStops(const map<string_view, StopRouteData>& stops_routing_data)
  {
    map<double, set<string_view>> longitude_sort;
    map<double, set<string_view>> latitude_sort;
    for (const auto& [stop_name, routing_data] : stops_routing_data) {
      const Sphere::Point position = routing_data.stop_position_;
      longitude_sort[position.longitude].insert(stop_name);
      latitude_sort[position.latitude].insert(stop_name);
    }

    // I feel a bit dirty for using such a strange data structure for this part
    // but if expected performance efficiency allows it, it will go
    const auto map_to_indexes = [&](const auto& stops_sorted_map, auto& uniform_mapping, auto& total_max) {
      for (const auto& [_, stop_names] : stops_sorted_map) {
        for (const auto& stop_name : stop_names) {
          vector<size_t> neighbour_places;
          const StopRouteData& route_data = stops_routing_data.at(stop_name);
          for (const auto& [indexed_stop_name, idx] : uniform_mapping) {
            const StopRouteData& indexed_route_data = stops_routing_data.at(indexed_stop_name);
            if (route_data.IsNeighboring(indexed_route_data)) {
              neighbour_places.push_back(idx);
            }
          }

          size_t idx_to_assign = 0;
          if (!neighbour_places.empty()) {
            const size_t max_idx = *max_element(begin(neighbour_places), end(neighbour_places));
            idx_to_assign = max_idx + 1;
          }
          uniform_mapping[stop_name] = idx_to_assign;
          total_max = max(total_max, idx_to_assign);
        }
      }
    };

    map_to_indexes(longitude_sort, x_uniform_mapping_, x_steps_);
    map_to_indexes(latitude_sort, y_uniform_mapping_, y_steps_);
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
    const double x_step = x_steps_ > 0 ? (width_ - 2 * padding_) / x_steps_ : 0;
    const double y_step = y_steps_ > 0 ? (height_ - 2 * padding_) / y_steps_ : 0;

    const size_t x_idx = x_uniform_mapping_.at(stop_name);
    const size_t y_idx = y_uniform_mapping_.at(stop_name);

    return { x_idx * x_step + padding_, height_ - padding_ - y_idx * y_step };
  }

private:
  map<string_view, size_t> x_uniform_mapping_;
  map<string_view, size_t> y_uniform_mapping_;

  size_t x_steps_ = 0;
  size_t y_steps_ = 0;

  double height_ = 0.;
  double width_ = 0.;
  double padding_ = 0.;
};

void
InterpolateBusRoute(const Descriptions::Bus& bus, map<string_view, StopRouteData>& stops_route_data)
{
  const vector<string>& routing_stops = bus.stops;

  vector<size_t> stop_indices;
  for (size_t i = 0; i < routing_stops.size(); ++i) {
    const string& stop = routing_stops[i];
    if (stop == bus.start_stop || stop == bus.end_stop) {
      stop_indices.push_back(i);
    } else {
      const StopRouteData& route_data = stops_route_data.at(stop);
      if (route_data.route_data_.size() > 1 || route_data.route_data_.at(bus.name).size() > 2) {
        stop_indices.push_back(i);
      }
    }
  }

  if (stop_indices.size() < 2) {
    return;
  }

  for (size_t i = 0; i < stop_indices.size() - 1; ++i) {
    const size_t idx_from = stop_indices[i];
    const size_t idx_to = stop_indices[i + 1];
    const size_t segment_size = idx_to - idx_from;

    const Sphere::Point& pt_from = stops_route_data.at(routing_stops[idx_from]).stop_position_;
    const Sphere::Point& pt_to = stops_route_data.at(routing_stops[idx_to]).stop_position_;
    const double lat_step = (pt_to.latitude - pt_from.latitude) / segment_size;
    const double lon_step = (pt_to.longitude - pt_from.longitude) / segment_size;

    for (size_t j = 0; j <= segment_size; ++j) {
      Sphere::Point& pt = stops_route_data.at(routing_stops[idx_from + j]).stop_position_;
      pt.latitude = pt_from.latitude + lat_step * j;
      pt.longitude = pt_from.longitude + lon_step * j;
    }
  }
}

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
MapRenderer::Init(std::map<string, Descriptions::Stop> stops, std::map<string, Descriptions::Bus> buses)
{
  stops_ = std::move(stops);
  buses_ = std::move(buses);
  InitMap();
}

string
MapRenderer::Render() const
{
  ostringstream os;
  doc_map_.Render(os);
  return os.str();
}

void
MapRenderer::InitMap()
{
  InitStopCoords();
  InitBusColors();
  for (const auto& layer : settings_.layers) {
    RenderLayer(layer);
  }
}

void
MapRenderer::InitStopCoords()
{
  map<string_view, StopRouteData> stops_route_data;
  for (const auto& [stop_name, stop] : stops_) {
    stops_route_data.emplace(stop_name, StopRouteData(stop, buses_));
  }

  for (const auto& [_, bus] : buses_) {
    InterpolateBusRoute(bus, stops_route_data);
  }

  UniformSvgMapper unimapper;
  unimapper.SetWidth(settings_.width)
    .SetHeight(settings_.height)
    .SetPaddint(settings_.padding)
    .SetStops(stops_route_data);

  StopPointMap point_map;
  for (auto& stop : stops_) {
    stop_coords_[stop.first] = unimapper(stop.first);
  }
}

void
MapRenderer::InitBusColors()
{
  size_t idx = 0;
  for (const auto& [bus_name, _] : buses_) {
    const Color color = settings_.color_palette[idx++ % settings_.color_palette.size()];
    bus_colors_[bus_name] = color;
  }
}

void
MapRenderer::RenderLayer(string_view layer)
{
  if (layer == "bus_lines") {
    RenderRouteLines();
  } else if (layer == "bus_labels") {
    RenderBusLabels();
  } else if (layer == "stop_points") {
    RenderStopSigns();
  } else if (layer == "stop_labels") {
    RenderStopLabels();
  }
}

void
MapRenderer::RenderRouteLines()
{
  for (const auto& [_, bus] : buses_) {
    vector<string_view> stops;
    stops.reserve(bus.stops.size());
    copy(begin(bus.stops), end(bus.stops), back_inserter(stops));
    RenderRouteLine(doc_map_, bus.name, stops);
  }
}
void
MapRenderer::RenderBusLabels()
{
  for (const auto& [_, bus] : buses_) {
    if (!bus.routing_stops.empty()) {
      const auto& start_stop = *begin(bus.routing_stops);
      const auto& end_stop = *rbegin(bus.routing_stops);
      RenderRouteLabel(doc_map_, bus.name, start_stop);
      if (start_stop != end_stop) {
        RenderRouteLabel(doc_map_, bus.name, end_stop);
      }
    }
  }
}
void
MapRenderer::RenderStopSigns()
{
  for (const auto& [stop, _] : stops_) {
    RenderStopSign(doc_map_, stop);
  }
}

void
MapRenderer::RenderStopLabels()
{
  for (const auto& [stop, _] : stops_) {
    RenderStopLabel(doc_map_, stop);
  }
}

std::string
MapRenderer::RenderRoute(const TransportRouter::RouteInfo& route_info) const
{
  Document route_doc(doc_map_);

  const double outer_margin = settings_.outer_margin;
  route_doc.Add(Rectangle()
                  .SetPosition(Point{ -outer_margin, -outer_margin })
                  .SetWidth(settings_.width + 2 * outer_margin)
                  .SetHeight(settings_.height + 2 * outer_margin)
                  .SetFillColor(settings_.underlayer_color));

  const auto route_data_array = BuildRouteDataArray(route_info);
  for (const auto& layer : settings_.layers) {
    RenderRouteLayer(layer, route_doc, route_data_array);
  }

  ostringstream os;
  route_doc.Render(os);
  return os.str();
}

void
MapRenderer::RenderRouteLayer(string_view layer, Document& route_doc, const RouteDataArray& route_data_array) const
{
  using RenderLayerFunc = void (MapRenderer::*)(Document&, const RouteDataArray&) const;
  static map<string_view, RenderLayerFunc> functions = { { "bus_lines", &MapRenderer::RenderRouteLines },
                                                         { "bus_labels", &MapRenderer::RenderRouteLabels },
                                                         { "stop_points", &MapRenderer::RenderRouteStopSigns },
                                                         { "stop_labels", &MapRenderer::RenderRouteStopLabels } };

  return (this->*functions.at(layer))(route_doc, route_data_array);
}

MapRenderer::RouteDataArray
MapRenderer::BuildRouteDataArray(const TransportRouter::RouteInfo& route_info) const
{
  RouteDataArray routes_data;

  auto next_wait_item_it = [](auto first, auto last) {
    return find_if(first, last, [](const auto& val) { return holds_alternative<RouteInfo::WaitItem>(val); });
  };

  const auto& route_items = route_info.items;
  for (auto item_it = begin(route_items); item_it != end(route_items);
       item_it = next_wait_item_it(next(item_it), end(route_items))) {

    RouteData route_data;

    const string_view stop_from = get<RouteInfo::WaitItem>(*item_it).stop_name;
    const size_t dist_to_end = distance(item_it, end(route_items));
    if (dist_to_end > 1) {
      const auto& bus_item = get<RouteInfo::BusItem>(*next(item_it));

      string_view stop_to = route_info.stop_to;
      if (dist_to_end > 2) {
        stop_to = get<RouteInfo::WaitItem>(*next(item_it, 2)).stop_name;
      }

      const auto& bus_stops = buses_.at(bus_item.bus_name).stops;
      const auto trip_stops =
        FindSubRange(begin(bus_stops), end(bus_stops), string(stop_from), string(stop_to), bus_item.span_count);

      route_data.bus = bus_item.bus_name;
      copy(begin(trip_stops), end(trip_stops), back_inserter(route_data.stops));

    } else {
      route_data.stops.push_back(stop_from);
    }
    routes_data.push_back(move(route_data));
  }
  return routes_data;
}


void
MapRenderer::RenderRouteLines(Document& route_doc, const RouteDataArray& route_data_array) const
{
  for (const auto& route_data : route_data_array) {
    RenderRouteLine(route_doc, route_data.bus, route_data.stops);
  }
}

void
MapRenderer::RenderRouteLabels(Document& route_doc, const RouteDataArray& route_data_array) const
{
  for (const auto& route_data : route_data_array) {
    if (route_data.stops.empty() || route_data.bus.empty()) {
      continue;
    }

    const string_view bus_name = route_data.bus;

    const auto& start_stop = *begin(route_data.stops);
    const auto& end_stop = *rbegin(route_data.stops);

    const Descriptions::Bus& bus_desc = buses_.at(string(bus_name));
    if (EqualsToOneOf(start_stop, bus_desc.start_stop, bus_desc.end_stop)) {
      RenderRouteLabel(route_doc, bus_name, start_stop);
    }
    if (end_stop != start_stop && EqualsToOneOf(end_stop, bus_desc.start_stop, bus_desc.end_stop)) {
      RenderRouteLabel(route_doc, bus_name, end_stop);
    }
  }
}

void
MapRenderer::RenderRouteStopSigns(Document& route_doc, const RouteDataArray& route_data_array) const
{
  for (const auto& route_data : route_data_array) {
    for (const auto& stop : route_data.stops) {
      RenderStopSign(route_doc, stop);
    }
  }
}

void
MapRenderer::RenderRouteStopLabels(Document& route_doc, const RouteDataArray& route_data_array) const
{
  if (route_data_array.empty()) {
    return;
  }

  vector<string_view> stops_to_render;
  stops_to_render.reserve(route_data_array.size() + 1);
  for (const auto& route_data : route_data_array) {
    stops_to_render.push_back(*begin(route_data.stops));
  }
  stops_to_render.push_back(*rbegin(rbegin(route_data_array)->stops));

  for (const auto& stop_name : stops_to_render) {
    RenderStopLabel(route_doc, stop_name);
  }
}

void
MapRenderer::RenderRouteLine(Document& doc, string_view bus, const vector<string_view>& stops) const
{
  if (stops.empty()) {
    return;
  }

  const Color color = bus_colors_.at(bus);
  Polyline route = Polyline()
                     .SetStrokeColor(color)
                     .SetStrokeWidth(settings_.line_width)
                     .SetStrokeLineCap("round")
                     .SetStrokeLineJoin("round");

  for (const auto& stop : stops) {
    route.AddPoint(stop_coords_.at(stop));
  }

  doc.Add(move(route));
}

void
MapRenderer::RenderRouteLabel(Document& doc, string_view bus, string_view stop) const
{
  const Color& color = bus_colors_.at(bus);
  const auto common = Text()
                        .SetPoint(stop_coords_.at(stop))
                        .SetOffset(settings_.bus_label_offset)
                        .SetFontSize(settings_.bus_label_font_size)
                        .SetFontFamily("Verdana")
                        .SetFontWeight("bold")
                        .SetData(string(bus));

  doc.Add(Text(common)
            .SetFillColor(settings_.underlayer_color)
            .SetStrokeColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap("round")
            .SetStrokeLineJoin("round"));

  doc.Add(Text(common).SetFillColor(color));
}

void
MapRenderer::RenderStopSign(Document& doc, std::string_view stop) const
{
  doc.Add(Circle().SetCenter(stop_coords_.at(stop)).SetRadius(settings_.stop_radius).SetFillColor("white"));
}

void
MapRenderer::RenderStopLabel(Document& doc, std::string_view stop) const
{
  const Point position = stop_coords_.at(stop);
  const Text common = Text()
                        .SetPoint(position)
                        .SetOffset(settings_.stop_label_offset)
                        .SetFontSize(settings_.stop_label_font_size)
                        .SetFontFamily("Verdana")
                        .SetData(string(stop));

  doc.Add(Text(common)
            .SetFillColor(settings_.underlayer_color)
            .SetStrokeColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap("round")
            .SetStrokeLineJoin("round"));

  doc.Add(Text(common).SetFillColor("black"));
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
  set_setting("outer_margin", outer_margin);
}

size_t
MapRenderer::Settings::LayerCount(std::string_view layer) const
{
  return count(begin(layers), end(layers), layer);
}

} // namespace Svg
