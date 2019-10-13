#include <algorithm>
#include <vector>

using namespace std;

template <typename T>
void Swap(T* first, T* second) {
  if (first != second) {
    auto tmp = move(*first);
    *first = move(*second);
    *second = move(tmp);
  }
}

template <typename T>
void SortPointers(vector<T*>& pointers) {
  sort(begin(pointers), end(pointers),
       [](auto const& lhs, auto const& rhs) { return *lhs < *rhs; });
}

template <typename T>
void ReversedCopy(T* source, size_t count, T* destination) {
  for (auto s = source, d = destination + count - 1; s != source + count;
       ++s, --d) {
    if (s >= destination && s < destination + count) {
      if (s < d) {
        Swap(s, d);
      }
    } else {
      *d = *s;
    }
  }
}