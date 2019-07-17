#include "svg.h"

namespace Svg {

///////////////////////////////////////////////////////
// Base value printers

std::ostream&
operator<<(std::ostream& os, const std::monostate&)
{
  return os << "none";
}

std::ostream&
operator<<(std::ostream& os, const Rgba& color_val)
{
  if (!color_val.alpha) {
    return os << "rgb(" << color_val.red << "," << color_val.green << "," << color_val.blue << ")";
  } else {
    return os << "rgba(" << color_val.red << "," << color_val.green << "," << color_val.blue << "," << *color_val.alpha
              << ")";
  }
}

std::ostream&
operator<<(std::ostream& os, const Color& color)
{
  return std::visit([&os](const auto& color_val) -> std::ostream& { return os << color_val; }, color.GetBase());
}

std::ostream&
operator<<(std::ostream& os, const Point& pt)
{
  return os << pt.x << "," << pt.y;
}

constexpr auto QUOTE = '"';
constexpr auto EQUALS = '=';

template<typename T>
void
PrintProperty(std::ostream& os, std::string_view name, const T& val)
{
  os << name << EQUALS << QUOTE << val << QUOTE << " ";
}

template<typename It>
void
PrintProperty(std::ostream& os, std::string_view name, It first, It last, std::string_view delimiter = " ")
{
  os << name << EQUALS << QUOTE;
  for (auto it = first; it != last; it = std::next(it)) {
    os << *it << delimiter;
  }
  os << QUOTE << " ";
}

///////////////////////////////////////////////////////
// Object methods impl

void
Object::Print(std::ostream& os) const
{
  os << "<" << GetName() << " ";
  PrintProperties(os);
  os << "/>";
}

template<typename T>
void
BaseObject<T>::PrintProperties(std::ostream& os) const
{
  PrintProperty(os, "fill", fill_color_);
  PrintProperty(os, "stroke", stroke_color_);
  PrintProperty(os, "stroke-width", stroke_width_);
  if (stroke_linecap_) {
    PrintProperty(os, "stroke-linecap", *stroke_linecap_);
  }
  if (stroke_linejoin_) {
    PrintProperty(os, "stroke-linejoin", *stroke_linejoin_);
  }
}

template<typename T>
T&
BaseObject<T>::SetFillColor(const Color& color)
{
  fill_color_ = color;
  return static_cast<T&>(*this);
}
template<typename T>
T&
BaseObject<T>::SetStrokeColor(const Color& color)
{
  stroke_color_ = color;
  return static_cast<T&>(*this);
}
template<typename T>
T&
BaseObject<T>::SetStrokeWidth(double stroke_width)
{
  stroke_width_ = stroke_width;
  return static_cast<T&>(*this);
}
template<typename T>
T&
BaseObject<T>::SetStrokeLineCap(const std::string& stroke_linecap)
{
  stroke_linecap_ = stroke_linecap;
  return static_cast<T&>(*this);
}
template<typename T>
T&
BaseObject<T>::SetStrokeLineJoin(const std::string& stroke_linejoin)
{
  stroke_linejoin_ = stroke_linejoin;
  return static_cast<T&>(*this);
}

///////////////////////////////////////////////////////
// Circle methods impl

void
Circle::PrintProperties(std::ostream& os) const
{
  PrintProperty(os, "cx", center_.x);
  PrintProperty(os, "cy", center_.y);
  PrintProperty(os, "r", radius_);

  BaseObject::PrintProperties(os);
}
const std::string&
Circle::GetName() const
{
  static std::string name = "circle";
  return name;
}

Circle&
Circle::SetCenter(const Point& center)
{
  center_ = center;
  return *this;
}
Circle&
Circle::SetRadius(double radius)
{
  radius_ = radius;
  return *this;
}

///////////////////////////////////////////////////////
// Polyline methods impl

void
Polyline::PrintProperties(std::ostream& os) const
{
  PrintProperty(os, "points", points_.begin(), points_.end());

  BaseObject::PrintProperties(os);
}
const std::string&
Polyline::GetName() const
{
  static std::string name = "polyline";
  return name;
}

Polyline&
Polyline::AddPoint(const Point& pt)
{
  points_.push_back(pt);
  return *this;
}

///////////////////////////////////////////////////////
// Text methods impl

void
Text::Print(std::ostream& os) const
{
  os << "<" << GetName() << " ";
  PrintProperties(os);
  os << ">" << data_ << "</" << GetName() << ">";
}

void
Text::PrintProperties(std::ostream& os) const
{
  PrintProperty(os, "x", position_.x);
  PrintProperty(os, "y", position_.y);
  PrintProperty(os, "dx", offset_.x);
  PrintProperty(os, "dy", offset_.y);
  PrintProperty(os, "font-size", font_size_);
  if (font_family_) {
    PrintProperty(os, "font-family", *font_family_);
  }

  BaseObject::PrintProperties(os);
}
const std::string&
Text::GetName() const
{
  static std::string name = "text";
  return name;
}

Text&
Text::SetPoint(const Point& pt)
{
  position_ = pt;
  return *this;
}
Text&
Text::SetOffset(const Point& pt)
{
  offset_ = pt;
  return *this;
}
Text&
Text::SetFontSize(uint32_t font_size)
{
  font_size_ = font_size;
  return *this;
}
Text&
Text::SetFontFamily(const std::string& font_family)
{
  font_family_ = font_family;
  return *this;
}
Text&
Text::SetData(const std::string& data)
{
  data_ = data;
  return *this;
}

///////////////////////////////////////////////////////
// Document methods impl

template<typename T>
void
Document::Add(T obj)
{
  objects_.emplace_back(std::make_unique<T>(std::move(obj)));
}

void
Document::Render(std::ostream& os)
{
  os << R"(<?xml version="1.0" encoding="UTF-8" ?>)";
  os << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)";

  for (const auto& object : objects_) {
    object->Print(os);
  }

  os << R"(</svg>)";
}

} // namespace Svg
