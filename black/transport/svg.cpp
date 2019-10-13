#include "svg.h"

namespace Svg {

///////////////////////////////////////////////////////
// Object methods impl

void
Object::Print(std::ostream& os) const
{
  os << "<" << GetName() << " ";
  PrintProperties(os);
  os << "/>";
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
  if (font_weight_) {
    PrintProperty(os, "font-weight", *font_weight_);
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
Text::SetFontWeight(const std::string& font_weight)
{
  font_weight_ = font_weight;
  return *this;
}

Text&
Text::SetData(const std::string& data)
{
  data_ = data;
  return *this;
}

///////////////////////////////////////////////////////
// Rectangle methods impl

void
Rectangle::PrintProperties(std::ostream& os) const
{
  PrintProperty(os, "x", position_.x);
  PrintProperty(os, "y", position_.y);
  PrintProperty(os, "width", width_);
  PrintProperty(os, "height", height_);

  BaseObject::PrintProperties(os);
}

const std::string&
Rectangle::GetName() const
{
  static std::string name = "rect";
  return name;
}

Rectangle&
Rectangle::SetPosition(const Point& pt)
{
  position_ = pt;
  return *this;
}

Rectangle&
Rectangle::SetWidth(double width)
{
  width_ = width;
  return *this;
}

Rectangle&
Rectangle::SetHeight(double height)
{
  height_ = height;
  return *this;
}

///////////////////////////////////////////////////////
// Document methods impl

void
Document::Render(std::ostream& os) const
{
  os << R"(<?xml version="1.0" encoding="UTF-8" ?>)";
  os << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)";

  for (const auto& object : objects_) {
    object->Print(os);
  }

  os << R"(</svg>)";
}

///////////////////////////////////////////////////////
// Compound methods impl

void
Compound::Print(std::ostream& os) const
{
  for (const auto& object : objects_) {
    object->Print(os);
  }
}

void
Compound::PrintProperties(std::ostream&) const
{}

const std::string&
Compound::GetName() const
{
  static std::string dummy;
  return dummy;
}

} // namespace Svg
