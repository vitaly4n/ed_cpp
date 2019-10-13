#include "game_object.h"
#include "geo2d.h"

class Unit : public GameObject
{
public:
  explicit Unit(geo2d::Point position)
    : point_{ position }
  {}

  bool Collide(const GameObject& that) const override;
  bool CollideWith(const Unit& that) const override;
  bool CollideWith(const Building& that) const override;
  bool CollideWith(const Tower& that) const override;
  bool CollideWith(const Fence& that) const override;

  const geo2d::Point& geo_data() const { return point_; }

private:
  geo2d::Point point_;
};

class Building : public GameObject
{
public:
  explicit Building(geo2d::Rectangle geometry)
    : rectangle_{ geometry }
  {}

  bool Collide(const GameObject& that) const override;
  bool CollideWith(const Unit& that) const override;
  bool CollideWith(const Building& that) const override;
  bool CollideWith(const Tower& that) const override;
  bool CollideWith(const Fence& that) const override;

  const geo2d::Rectangle& geo_data() const { return rectangle_; }

private:
  geo2d::Rectangle rectangle_;
};

class Tower : public GameObject
{
public:
  explicit Tower(geo2d::Circle geometry)
    : circle_{ geometry }
  {}

  bool Collide(const GameObject& that) const override;
  bool CollideWith(const Unit& that) const override;
  bool CollideWith(const Building& that) const override;
  bool CollideWith(const Tower& that) const override;
  bool CollideWith(const Fence& that) const override;

  const geo2d::Circle& geo_data() const { return circle_; }

private:
  geo2d::Circle circle_;
};

class Fence : public GameObject
{
public:
  explicit Fence(geo2d::Segment geometry)
    : segment_{ geometry }
  {}

  bool Collide(const GameObject& that) const override;
  bool CollideWith(const Unit& that) const override;
  bool CollideWith(const Building& that) const override;
  bool CollideWith(const Tower& that) const override;
  bool CollideWith(const Fence& that) const override;

  const geo2d::Segment& geo_data() const { return segment_; }

private:
  geo2d::Segment segment_;
};

bool
Collide(const GameObject& first, const GameObject& second)
{
  return first.Collide(second);
}

#define GAME_OBJECT_COLLIDE_WITH(GameTypeFirst, GameTypeSecond)                \
  bool GameTypeFirst::CollideWith(const GameTypeSecond& that) const            \
  {                                                                            \
    return geo2d::Collide(geo_data(), that.geo_data());                        \
  }

#define GAME_OBJECT_DESPATCH_DEF(GameObjType)                                  \
  bool GameObjType::Collide(const GameObject& that) const                      \
  {                                                                            \
    return that.CollideWith(*this);                                            \
  }                                                                            \
  GAME_OBJECT_COLLIDE_WITH(GameObjType, Unit)                                  \
  GAME_OBJECT_COLLIDE_WITH(GameObjType, Building)                              \
  GAME_OBJECT_COLLIDE_WITH(GameObjType, Tower)                                 \
  GAME_OBJECT_COLLIDE_WITH(GameObjType, Fence)

GAME_OBJECT_DESPATCH_DEF(Unit)
GAME_OBJECT_DESPATCH_DEF(Building)
GAME_OBJECT_DESPATCH_DEF(Tower)
GAME_OBJECT_DESPATCH_DEF(Fence)
