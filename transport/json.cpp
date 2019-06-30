#include "json.h"

#include <cmath>

using namespace std;

namespace {

string
FormIndentation(int level)
{
  string res;
  for (int i = 0; i < level; ++i) {
    res += "  ";
  }
  return res;
}

class AutoScope
{
public:
  AutoScope() = default;
  AutoScope LevelUp() const { return AutoScope(*this); }

  AutoScope(AutoScope&&) = delete;
  AutoScope& operator=(AutoScope&&) = delete;

  string GetIndentation() const { return FormIndentation(level_); }

private:
  AutoScope(const AutoScope& other) { level_ = other.level_ + 1; }
  AutoScope& operator=(const AutoScope& other)
  {
    level_ = other.level_ + 1;
    return *this;
  }

  int level_ = 0;
};
}

namespace Json {

Document::Document(Node root)
  : root(move(root))
{}

const Node&
Document::GetRoot() const
{
  return root;
}

Node&
Document::GetRoot()
{
  return root;
}

Node
LoadNode(istream& input);

ostream&
UnloadNode(ostream& output, const Node& node, const AutoScope& scope);

Node
LoadArray(istream& input)
{
  vector<Node> result;

  for (char c; input >> c && c != ']';) {
    if (c != ',') {
      input.putback(c);
    }
    result.push_back(LoadNode(input));
  }

  return Node(move(result));
}

Node
LoadNumeral(istream& input)
{
  int result = 0;

  if (input.peek() == 't') {
    input.ignore(4);
    return Node(true);
  } else if (input.peek() == 'f') {
    input.ignore(5);
    return Node(false);
  }

  int result_sign = 1;
  if (input.peek() == '-') {
    input.ignore();
    result_sign = -1;
  }

  while (isdigit(input.peek())) {
    result *= 10;
    result += input.get() - '0';
  }
  if (input.peek() == '.') {
    input.ignore();

    double fraction = 0.;
    int cur_order = -1;
    while (isdigit(input.peek())) {
      fraction += pow(10, cur_order) * (input.get() - '0');
      --cur_order;
    }
    return Node(result_sign * (fraction + result));
  }

  return Node(result * result_sign);
}

Node
LoadString(istream& input)
{
  string line;
  getline(input, line, '"');
  return Node(move(line));
}

Node
LoadDict(istream& input)
{
  map<string, Node> result;

  for (char c; input >> c && c != '}';) {
    if (c == ',') {
      input >> c;
    }

    string key = LoadString(input).AsString();
    input >> c;
    result.emplace(move(key), LoadNode(input));
  }

  return Node(move(result));
}

Node
LoadNode(istream& input)
{
  char c;
  input >> c;

  if (c == '[') {
    return LoadArray(input);
  } else if (c == '{') {
    return LoadDict(input);
  } else if (c == '"') {
    return LoadString(input);
  } else {
    input.putback(c);
    return LoadNumeral(input);
  }
}

ostream&
UnloadDict(ostream& output, const Node& node, const AutoScope& scope)
{
  output << "{";
  const auto& obj = node.AsMap();

  auto internal_scope = scope.LevelUp();
  for (auto it = begin(obj); it != end(obj); it = next(it)) {
    if (it == begin(obj)) {
      output << "\n" << internal_scope.GetIndentation();
    }

    const auto& [key, val] = *it;
    output << "\"" << key << "\" : ";
    UnloadNode(output, val, internal_scope);

    if (next(it) != end(obj)) {
      output << ",\n" << internal_scope.GetIndentation();
    } else {
      output << "\n" << scope.GetIndentation();
    }
  }
  output << "}";
  return output;
}

ostream&
UnloadArray(ostream& output, const Node& node, const AutoScope& scope)
{
  output << "[";
  const auto& arr = node.AsArray();

  auto internal_scope = scope.LevelUp();
  for (auto it = begin(arr); it != end(arr); it = next(it)) {
    if (it == begin(arr)) {
      output << "\n" << internal_scope.GetIndentation();
    }

    UnloadNode(output, *it, internal_scope);

    if (next(it) != end(arr)) {
      output << ",\n" << internal_scope.GetIndentation();
    } else {
      output << "\n" << scope.GetIndentation();
    }
  }
  output << "]";
  return output;
}

ostream&
UnloadString(ostream& output, const Node& node, const AutoScope& scope)
{
  output << "\"" << node.AsString() << "\"";
  return output;
}

ostream&
UnloadInteger(ostream& output, const Node& node, const AutoScope& scope)
{
  output << node.AsInt();
  return output;
}

ostream&
UnloadDouble(ostream& output, const Node& node, const AutoScope& scope)
{
  output << node.AsDouble();
  return output;
}

ostream&
UnloadNode(ostream& output, const Node& node, const AutoScope& scope)
{
  switch (node.GetType()) {
    case Node::eMap:
      UnloadDict(output, node, scope);
      break;
    case Node::eArray:
      UnloadArray(output, node, scope);
      break;
    case Node::eString:
      UnloadString(output, node, scope);
      break;
    case Node::eInt:
      UnloadInteger(output, node, scope);
      break;
    case Node::eDouble:
      UnloadDouble(output, node, scope);
      break;
    default:
      break;
  }
  return output;
}

Document
Load(istream& input)
{
  return Document{ LoadNode(input) };
}

ostream&
Unload(ostream& output, const Document& doc)
{
  return UnloadNode(output, doc.GetRoot(), AutoScope());
}
}
