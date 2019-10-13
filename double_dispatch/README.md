## Double dispatch

### Preface

Assume you write an engine for a classical PC strategy. You have:
- units: soldiers, workers etc
- buildings: plants, barracs etc
- towers and walls: a defince of a city

You are working on a task about placement new objects on the map. Assume you want to plant a new building. You start looking for a place on the map where it can be planted, while a GUI helps you in the task. If the building cannot be planted in the current position because some objects would intersect with it, GUI draws your buiding in red color.

Units are represeented as points on a plane, buildings - rectangles, towers - circles, walls - lines. Besides, every object in your game is derived from an abstract class *GameObject*:
```
class Unit;
class Building;
class Tower;
class Fence;

struct GameObject {
  virtual ~GameObject() = default;

  virtual bool Collide(const GameObject& that) const = 0;
  virtual bool CollideWith(const Unit& that) const = 0;
  virtual bool CollideWith(const Building& that) const = 0;
  virtual bool CollideWith(const Tower& that) const = 0;
  virtual bool CollideWith(const Fence& that) const = 0;
};
```

### Task

Your task is to implement function *bool Collide(const GameObject& first, const GameObject& second)* which check whether two given objects intersect.

A library for working with 2d geometrical objects is provided.

RTTI is not allowed.