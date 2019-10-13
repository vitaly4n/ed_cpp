
using namespace std;

template <typename T>
class LinkedList {
 public:
  struct Node {
    T value;
    Node* next = nullptr;
  };

  ~LinkedList() { 
    while (head) {
      auto new_head = head->next;
      delete head;
      head = new_head;
    }
  }

  void PushFront(const T& value) { 
    auto new_head = new Node{value, head};
    head = new_head;
  }

  void InsertAfter(Node* node, const T& value) {
    if (!node) {
      PushFront(value);
      return;
    } else {
      auto insert_node = new Node{value, node->next};
      node->next = insert_node;
    }  
  }
  void RemoveAfter(Node* node) {
    if (!node) {
      PopFront();
    } else if (node->next) {
      auto new_next = node->next->next;
      delete node->next;
      node->next = new_next;
    }
  }

  void PopFront() {
    if (head) {
      auto new_head = head->next;
      delete head;
      head = new_head;
    }
  }

  Node* GetHead() { return head; }
  const Node* GetHead() const { return head; }

 private:
  Node* head = nullptr;
};