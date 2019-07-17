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
  Text& SetData(const std::string& data);

private:
  Point position_;
  Point offset_;
  uint32_t font_size_ = 1;
  std::optional<std::string> font_family_;
  std::string data_;
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

} // namespace Svg
