#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#ifdef LOCAL_TEST

#include "test_runner.h"

#endif

namespace Svg {

struct Point
{
  double x = 0.;
  double y = 0.;
};

struct Rgb
{
  int red = 0;
  int green = 0;
  int blue = 0;
};

class Color : std::variant<std::monostate, std::string, Rgb>
{
public:
  using variant::variant;
  variant& GetBase() { return *this; }
  const variant& GetBase() const { return *this; }
};

const Color NoneColor = Color();

class ObjectCloneVisitor;
class ObjectMovingVisitor;

class Object;
using ObjectPtr = std::unique_ptr<Object>;
class Object
{
public:
  virtual ~Object() = default;

  virtual ObjectPtr Accept(ObjectCloneVisitor& visitor) const = 0;
  virtual ObjectPtr Accept(ObjectMovingVisitor& visitor) = 0;

  virtual void Print(std::ostream& os) const;
  virtual void PrintProperties(std::ostream& os) const = 0;
  virtual const std::string& GetName() const = 0;
};

template<typename T>
class BaseObject : public Object
{
public:
  void PrintProperties(std::ostream& os) const override;

  T& SetFillColor(const Color& color);
  T& SetStrokeColor(const Color& color);
  T& SetStrokeWidth(double stroke_width);
  T& SetStrokeLineCap(const std::string& stroke_linecap);
  T& SetStrokeLineJoin(const std::string& stroke_linejoin);

private:
  Color fill_color_ = NoneColor;
  Color stroke_color_ = NoneColor;
  double stroke_width_ = 1.;
  std::optional<std::string> stroke_linecap_;
  std::optional<std::string> stroke_linejoin_;
};

class Circle : public BaseObject<Circle>
{
public:
  ObjectPtr Accept(ObjectCloneVisitor& visitor) const override;
  ObjectPtr Accept(ObjectMovingVisitor& visitor) override;

  void PrintProperties(std::ostream& os) const override;
  const std::string& GetName() const override;

  Circle& SetCenter(const Point& center);
  Circle& SetRadius(double radius);

private:
  Point center_;
  double radius_ = 1.;
};

class Polyline : public BaseObject<Polyline>
{
public:
  ObjectPtr Accept(ObjectCloneVisitor& visitor) const override;
  ObjectPtr Accept(ObjectMovingVisitor& visitor) override;

  void PrintProperties(std::ostream& os) const override;
  const std::string& GetName() const override;

  Polyline& AddPoint(const Point& pt);

private:
  std::vector<Point> points_;
};

class Text : public BaseObject<Text>
{
public:
  ObjectPtr Accept(ObjectCloneVisitor& visitor) const override;
  ObjectPtr Accept(ObjectMovingVisitor& visitor) override;

  void Print(std::ostream& os) const override;
  void PrintProperties(std::ostream& os) const override;
  const std::string& GetName() const override;

  Text& SetPoint(const Point& pt);
  Text& SetOffset(const Point& pt);
  Text& SetFontSize(uint32_t font_size);
  Text& SetFontFamily(const std::string& font_family);
  Text& SetData(const std::string& data);

private:
  Point position_;
  Point offset_;
  uint32_t font_size_ = 1;
  std::optional<std::string> font_family_;
  std::string data_;
};

class ObjectCloneVisitor
{
public:
  ObjectPtr Visit(const Circle& obj);
  ObjectPtr Visit(const Polyline& obj);
  ObjectPtr Visit(const Text& obj);
};

class ObjectMovingVisitor
{
public:
  ObjectPtr Visit(Circle&& obj);
  ObjectPtr Visit(Polyline&& obj);
  ObjectPtr Visit(Text&& obj);
};

class Document
{
public:
  Document& Add(Object&& object);
  Document& Add(const Object& object);
  void Render(std::ostream& render);

private:
  std::vector<ObjectPtr> objects_;
};

///////////////////////////////////////////////////////
// Base value printers

std::ostream&
operator<<(std::ostream& os, const std::monostate&)
{
  return os << "none";
}

std::ostream&
operator<<(std::ostream& os, const Rgb& color_val)
{
  return os << "rgb(" << color_val.red << "," << color_val.green << "," << color_val.blue << ")";
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

ObjectPtr
Circle::Accept(ObjectCloneVisitor& visitor) const
{
  return visitor.Visit(*this);
}
ObjectPtr
Circle::Accept(ObjectMovingVisitor& visitor)
{
  return visitor.Visit(std::move(*this));
}

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

ObjectPtr
Polyline::Accept(ObjectCloneVisitor& visitor) const
{
  return visitor.Visit(*this);
}
ObjectPtr
Polyline::Accept(ObjectMovingVisitor& visitor)
{
  return visitor.Visit(std::move(*this));
}

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

ObjectPtr
Text::Accept(ObjectCloneVisitor& visitor) const
{
  return visitor.Visit(*this);
}
ObjectPtr
Text::Accept(ObjectMovingVisitor& visitor)
{
  return visitor.Visit(std::move(*this));
}

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
// ObjectCloneVisitor methods impl

ObjectPtr
ObjectCloneVisitor::Visit(const Circle& obj)
{
  return std::make_unique<Circle>(obj);
}
ObjectPtr
ObjectCloneVisitor::Visit(const Polyline& obj)
{
  return std::make_unique<Polyline>(obj);
}
ObjectPtr
ObjectCloneVisitor::Visit(const Text& obj)
{
  return std::make_unique<Text>(obj);
}

///////////////////////////////////////////////////////
// ObjectMovingVisitor methods impl

ObjectPtr
ObjectMovingVisitor::Visit(Circle&& obj)
{
  return std::make_unique<Circle>(std::move(obj));
}
ObjectPtr
ObjectMovingVisitor::Visit(Polyline&& obj)
{
  return std::make_unique<Polyline>(std::move(obj));
}
ObjectPtr
ObjectMovingVisitor::Visit(Text&& obj)
{
  return std::make_unique<Text>(std::move(obj));
}

///////////////////////////////////////////////////////
// Document methods impl

Document&
Document::Add(Object&& object)
{
  ObjectMovingVisitor visitor;
  objects_.emplace_back(object.Accept(visitor));
  return *this;
}

Document&
Document::Add(const Object& object)
{
  ObjectCloneVisitor visitor;
  objects_.emplace_back(object.Accept(visitor));
  return *this;
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

#ifdef LOCAL_TEST

void
simple_test()
{
  Svg::Document svg;

  svg.Add(Svg::Polyline{}
            .SetStrokeColor(Svg::Rgb{ 140, 198, 63 }) // soft green
            .SetStrokeWidth(16)
            .SetStrokeLineCap("round")
            .AddPoint({ 50, 50 })
            .AddPoint({ 250, 250 }));

  for (const auto& point : { Svg::Point{ 50, 50 }, Svg::Point{ 250, 250 } }) {
    svg.Add(Svg::Circle{}.SetFillColor("white").SetRadius(6).SetCenter(point));
  }

  svg.Add(Svg::Text{}
            .SetPoint({ 50, 50 })
            .SetOffset({ 10, -10 })
            .SetFontSize(20)
            .SetFontFamily("Verdana")
            .SetFillColor("black")
            .SetData("C"));
  svg.Add(Svg::Text{}
            .SetPoint({ 250, 250 })
            .SetOffset({ 10, -10 })
            .SetFontSize(20)
            .SetFontFamily("Verdana")
            .SetFillColor("black")
            .SetData("C++"));

  std::string ref =
    R"xx(<?xml version="1.0" encoding="UTF-8" ?><svg xmlns="http://www.w3.org/2000/svg" version="1.1"><polyline points="50,50 250,250 " fill="none" stroke="rgb(140,198,63)" stroke-width="16" stroke-linecap="round" /><circle cx="50" cy="50" r="6" fill="white" stroke="none" stroke-width="1" /><circle cx="250" cy="250" r="6" fill="white" stroke="none" stroke-width="1" /><text x="50" y="50" dx="10" dy="-10" font-size="20" font-family="Verdana" fill="black" stroke="none" stroke-width="1" >C</text><text x="250" y="250" dx="10" dy="-10" font-size="20" font-family="Verdana" fill="black" stroke="none" stroke-width="1" >C++</text></svg>)xx";
  std::stringstream out;
  svg.Render(out);
  ASSERT_EQUAL(out.str(), ref);
}

int
main(int, char*[])
{
  TestRunner tr;
  RUN_TEST(tr, simple_test);
  return 0;
}

#endif
