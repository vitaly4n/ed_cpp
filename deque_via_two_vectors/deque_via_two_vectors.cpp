#include <cstdlib>
#include <vector>

using namespace std;

template <typename T>
class Deque {
 public:
  bool Empty() const { return front_elts_.empty() && back_elts_.empty(); }
  size_t Size() const { return front_elts_.size() + back_elts_.size(); }

  void PushFront(const T& val) { front_elts_.push_back(val); }
  void PushBack(const T& val) { back_elts_.push_back(val); }

  T& At(size_t idx) {
    auto const front_size = front_elts_.size();
    if (idx < front_size) {
      return front_elts_.at(front_size - idx - 1);
    } else {
      return back_elts_.at(idx - front_size);
    }
  }

  T const& At(size_t idx) const {
    auto const front_size = front_elts_.size();
    if (idx < front_size) {
      return front_elts_.at(front_size - idx - 1);
    } else {
      return back_elts_.at(idx - front_size);
    }
  }

  T& operator[](size_t idx) { return At(idx); }
  T const& operator[](size_t idx) const { return At(idx); }

  T& Front() { return At(0); }
  T const& Front() const { return At(0); }

  T& Back() { return At(Size() - 1); }
  T const& Back() const { return At(Size() - 1); }


 private:
  vector<T> front_elts_;
  vector<T> back_elts_;
};
