#include <deque>
using namespace std;

struct Node
{
  Node(int v, Node* p)
    : value(v)
    , parent(p)
  {}

  int value;
  Node* left = nullptr;
  Node* right = nullptr;
  Node* parent;
};

Node*
Next(Node* node)
{
  if (node->right) {
    node = node->right;
    while (node->left)
      node = node->left;
    return node;
  }

  for (; node; node = node->parent) {
    if (node->parent && node->parent->left == node)
      return node->parent;
  }

  return nullptr;
}