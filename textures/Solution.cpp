#include "Common.h"

#include <algorithm>
#include <memory>
#include <numeric>
#include <utility>

using namespace std;

namespace {

Size
GetImageSize(const Image& image)
{
  if (image.empty()) {
    return { 0, 0 };
  }
  return { int(image.front().size()), int(image.size()) };
}

void
PatchImage(Image& target, const Image& source, Point start)
{
  const auto target_size = GetImageSize(target);
  const auto source_size = GetImageSize(source);
  if (target_size.width == 0 || target_size.height == 0 ||
      source_size.width == 0 || source_size.height == 0) {
    return;
  }

  const Point target_end{ target_size.width, target_size.height };
  const Point source_end{ start.x + source_size.width,
                          start.y + source_size.height };
  const Point end{ min(target_end.x, source_end.x),
                   min(target_end.y, source_end.y) };

  for (auto x = start.x; x < end.x; ++x) {
    for (auto y = start.y; y < end.y; ++y) {
      if (auto ch = source[y - start.y][x - start.x]) {
        if (target[y][x]) {
          target[y][x] = ch;
        }
      }
    }
  }
}

class DefaultShape : public IShape
{
public:
  void SetPosition(Point position) override { position_ = position; }
  Point GetPosition() const override { return position_; }

  void SetSize(Size size) override { size_ = size; }
  Size GetSize() const override { return size_; }

  void SetTexture(std::shared_ptr<ITexture> texture) override
  {
    texture_ = move(texture);
  }
  ITexture* GetTexture() const override { return texture_.get(); }

  void Draw(Image& image) const override
  {
    const string line(size_.width, 0);
    Image shape_image(size_.height, line);
    for (auto x = 0; x < size_.width; ++x) {
      for (auto y = 0; y < size_.height; ++y) {
        if (IsPointInsideShape({ x, y })) {
          shape_image[y][x] = '.';
        }
      }
    }

    if (texture_.get()) {
      PatchImage(shape_image, texture_->GetImage(), { 0, 0 });
    }
    PatchImage(image, shape_image, position_);
  }

protected:
  virtual bool IsPointInsideShape(const Point& point) const = 0;

  shared_ptr<ITexture> texture_;
  Size size_;
  Point position_;
};

class Rectangle : public DefaultShape
{
public:
  std::unique_ptr<IShape> Clone() const override
  {
    return make_unique<Rectangle>(*this);
  }

protected:
  bool IsPointInsideShape(const Point& point) const override
  {
    return point.x < position_.x + size_.width &&
           point.y < position_.y + size_.height;
  }
};

class Ellipse : public DefaultShape
{
public:
  std::unique_ptr<IShape> Clone() const override
  {
    return make_unique<Ellipse>(*this);
  }

protected:
  bool IsPointInsideShape(const Point& point) const override
  {
    return IsPointInEllipse(point, size_);
  }
};

} // namespace

unique_ptr<IShape>
MakeShape(ShapeType shape_type)
{
  if (shape_type == ShapeType::Rectangle)
    return make_unique<Rectangle>();
  else if (shape_type == ShapeType::Ellipse)
    return make_unique<Ellipse>();
  return nullptr;
}
