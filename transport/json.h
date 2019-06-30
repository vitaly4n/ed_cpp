#pragma once

#include <cassert>
#include <istream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace Json {

class Node
  : std::variant<std::vector<Node>,
                 std::map<std::string, Node>,
                 int,
                 double,
                 std::string,
                 bool>
{
public:
  enum Type
  {
    eArray = 0,
    eMap,
    eInt,
    eDouble,
    eString,
    eBool
  };

  using variant::variant;

  const auto& AsArray() const
  {
    assert(index() == eArray);
    return std::get<std::vector<Node>>(*this);
  }
  const auto& AsMap() const
  {
    assert(index() == eMap);
    return std::get<std::map<std::string, Node>>(*this);
  }
  int AsInt() const
  {
    assert(index() == eInt);
    return std::get<int>(*this);
  }
  double AsDouble() const
  {
    assert(index() == eInt || index() == eDouble);
    if (index() == 3) {
      return std::get<double>(*this);
    } else {
      return static_cast<double>(std::get<int>(*this));
    }
  }
  const auto& AsString() const
  {
    assert(index() == eString);
    return std::get<std::string>(*this);
  }
  bool AsBool() const 
  { 
    assert(index() == eBool);
    return std::get<bool>(*this);
  }

  Type GetType() const { return static_cast<Type>(index()); }
};

class Document
{
public:
  explicit Document(Node root);

  const Node& GetRoot() const;
  Node& GetRoot();

private:
  Node root;
};

Document
Load(std::istream& input);

std::ostream&
Unload(std::ostream& output, const Document& doc);
}
