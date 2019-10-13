#include <cstdlib>
#include <vector>

using namespace std;

template <typename Iterator>
class IteratorRange {
 public:
  IteratorRange(Iterator first, Iterator last) : first_{first}, last_{last} {}

  Iterator begin() const { return first_; }
  Iterator end() const { return last_; }

  size_t size() const { return distance(first_, last_); }

 private:
  Iterator first_;
  Iterator last_;
};

template <typename It>
class Paginator {
 public:
  using Data = vector<IteratorRange<It>>;
  using iterator = typename Data::iterator;
  using const_iterator = typename Data::const_iterator;

  Paginator(It first, It last, size_t page_size) : page_size_{page_size} {
    for (auto it = first; it != last;) {
      auto page_end = distance(it, last) > page_size ? it + page_size : last;
      pages_.emplace_back(it, page_end);
      it = page_end;
    }
  }

  iterator begin() { return std::begin(pages_); }
  iterator end() { return std::end(pages_); }

  const_iterator begin() const { return std::begin(pages_); }
  const_iterator end() const { return std::end(pages_); }

  size_t page_size() const { return page_size_; }
  size_t size() const { return pages_.size(); }

 private:
  Data pages_;

  size_t page_size_ = 0;
};

template <typename C>
auto Paginate(C& c, size_t page_size) {
  return Paginator{begin(c), end(c), page_size};
}
