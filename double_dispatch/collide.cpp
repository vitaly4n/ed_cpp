#include "game_object.h"
#include "geo2d.h"

class Unit : public GameObject
{
public:
  explicit Unit(geo2d::Point position) {}

  bool Collide(const GameObject& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Unit& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Building& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Tower& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Fence& that) const override
  { // TODO
    return false;
  }
};

class Building : public GameObject
{
public:
  explicit Building(geo2d::Rectangle geometry) {}

  bool Collide(const GameObject& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Unit& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Building& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Tower& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Fence& that) const override
  { // TODO
    return false;
  }
};

class Tower : public GameObject
{
public:
  explicit Tower(geo2d::Circle geometry) {}

  bool Collide(const GameObject& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Unit& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Building& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Tower& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Fence& that) const override
  { // TODO
    return false;
  }
};

class Fence : public GameObject
{
public:
  explicit Fence(geo2d::Segment geometry) {}

  bool Collide(const GameObject& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Unit& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Building& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Tower& that) const override
  { // TODO
    return false;
  }
  bool CollideWith(const Fence& that) const override
  { // TODO
    return false;
  }
};

bool
Collide(const GameObject& first, const GameObject& second)
{
  return false;
}
