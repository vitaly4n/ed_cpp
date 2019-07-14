
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

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

  void Print(std::ostream& os) const;
  virtual void PrintProperties(std::ostream& os) const;
  virtual const std::string& GetName() const = 0;

  Object& SetFillColor(const Color& color);
  Object& SetStrokeColor(const Color& color);
  Object& SetStrokeWidth(double stroke_width);
  Object& SetStrokeLineCap(const std::string& stroke_linecap);
  Object& SetStrokeLineJoin(const std::string& stroke_linejoin);

private:
  Color fill_color_ = NoneColor;
  Color stroke_color_ = NoneColor;
  double stroke_width_ = 1.;
  std::optional<std::string> stroke_linecap_;
  std::optional<std::string> stroke_linejoin_;
};

class Circle : public Object
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

class Polyline : public Object
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

class Text : public Object
{
public:
  ObjectPtr Accept(ObjectCloneVisitor& visitor) const override;
  ObjectPtr Accept(ObjectMovingVisitor& visitor) override;

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

///////////////////////////////////////////////////////
// Object methods impl

void
Object::Print(std::ostream& os) const
{
  os << "<" << GetName() << " ";
  PrintProperties(os);
  os << "/>";
}
void
Object::PrintProperties(std::ostream& os) const
{
  // TODO
}

Object&
Object::SetFillColor(const Color& color)
{
  fill_color_ = color;
  return *this;
}
Object&
Object::SetStrokeColor(const Color& color)
{
  stroke_color_ = color;
  return *this;
}
Object&
Object::SetStrokeWidth(double stroke_width)
{
  stroke_width_ = stroke_width;
  return *this;
}
Object&
Object::SetStrokeLineCap(const std::string& stroke_linecap)
{
  stroke_linecap_ = stroke_linecap;
  return *this;
}
Object&
Object::SetStrokeLineJoin(const std::string& stroke_linejoin)
{
  stroke_linejoin_ = stroke_linejoin;
  return *this;
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
  // TODO
  Object::PrintProperties(os);
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
  // TODO
  Object::PrintProperties(os);
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
Text::PrintProperties(std::ostream& os) const
{
  // TODO
  Object::PrintProperties(os);
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
  for (const auto& object : objects_) {
    object->Print(os);
  }
}

} // namespace Svg

#ifdef LOCAL_TEST
int
main(int, char*[])
{}

#endif
