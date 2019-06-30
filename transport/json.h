#pragma once

#include <cassert>
#include <istream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <iostream>

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
    if (index() != eArray) {
      std::cerr << "Attempt to read array with " << index();
    }
    return std::get<std::vector<Node>>(*this);
  }
  const auto& AsMap() const
  {
    if (index() != eMap) {
      std::cerr << "Attempt to read map with " << index();
    }
    return std::get<std::map<std::string, Node>>(*this);
  }
  int AsInt() const
  {
    if (index() != eInt) {
      std::cerr << "Attempt to read int with " << index();
    }
    return std::get<int>(*this);
  }
  double AsDouble() const
  {
    if (index() != eInt && index() != eDouble) {
      std::cerr << "Attempt to read double with " << index();
    }
    if (index() == 3) {
      return std::get<double>(*this);
    } else {
      return static_cast<double>(std::get<int>(*this));
    }
  }
  const auto& AsString() const
  {
    if (index() != eString) {
      std::cerr << "Attempt to read string with " << index();
    }
    return std::get<std::string>(*this);
  }
  bool AsBool() const 
  { 
    if (index() != eBool) {
      std::cerr << "Attempt to read bool with " << index();
    }
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
