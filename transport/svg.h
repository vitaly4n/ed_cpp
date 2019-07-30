#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace Svg {

struct Point
{
  double x = 0.;
  double y = 0.;
};

struct Rgba
{
  int red = 0;
  int green = 0;
  int blue = 0;
  std::optional<double> alpha;
};

class Color : std::variant<std::monostate, std::string, Rgba>
{
public:
  using variant::variant;
  variant& GetBase() { return *this; }
  const variant& GetBase() const { return *this; }
};

const Color NoneColor = Color();

class Object;
using ObjectPtr = std::unique_ptr<Object>;
class Object
{
public:
  Object() = default;
  Object(const Object&) = default;
  Object(Object&&) = default;
  Object& operator=(const Object&) = default;
  Object& operator=(Object&&) = default;

  virtual ~Object() = default;

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
  void PrintProperties(std::ostream& os) const override;
  const std::string& GetName() const override;

  Polyline& AddPoint(const Point& pt);

private:
  std::vector<Point> points_;
};

class Text : public BaseObject<Text>
{
public:
  void Print(std::ostream& os) const override;
  void PrintProperties(std::ostream& os) const override;
  const std::string& GetName() const override;

  Text& SetPoint(const Point& pt);
  Text& SetOffset(const Point& pt);
  Text& SetFontSize(uint32_t font_size);
  Text& SetFontFamily(const std::string& font_family);
  Text& SetFontWeight(const std::string& font_weight);
  Text& SetData(const std::string& data);

private:
  Point position_;
  Point offset_;
  uint32_t font_size_ = 1;
  std::optional<std::string> font_family_;
  std::optional<std::string> font_weight_;
  std::string data_;
};

class Compound : public Object
{
public:
  template<typename T>
  void Add(T obj);

  void Print(std::ostream& os) const override;
  void PrintProperties(std::ostream&) const override;
  const std::string& GetName() const override;

private:
  std::vector<ObjectPtr> objects_;
};

class Document
{
public:
  template<typename T>
  void Add(T obj);

  void Render(std::ostream& render);

private:
  std::vector<ObjectPtr> objects_;
};

///////////////////////////////////////////////////////
// template stuff

inline std::ostream&
operator<<(std::ostream& os, const std::monostate&)
{
  return os << "none";
}

inline std::ostream&
operator<<(std::ostream& os, const Rgba& color_val)
{
  if (!color_val.alpha) {
    return os << "rgb(" << color_val.red << "," << color_val.green << "," << color_val.blue << ")";
  } else {
    return os << "rgba(" << color_val.red << "," << color_val.green << "," << color_val.blue << "," << *color_val.alpha
              << ")";
  }
}

inline std::ostream&
operator<<(std::ostream& os, const Color& color)
{
  return std::visit([&os](const auto& color_val) -> std::ostream& { return os << color_val; }, color.GetBase());
}

inline std::ostream&
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

template<typename T>
void Compound::Add(T obj)
{
  objects_.push_back(std::make_unique<T>(std::move(obj)));
}

template<typename T>
void
Document::Add(T obj)
{
  objects_.emplace_back(std::make_unique<T>(std::move(obj)));
}

} // namespace Svg
