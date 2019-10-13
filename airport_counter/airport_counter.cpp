#include <algorithm>
#include <array>
#include <cstdlib>
#include <vector>

using namespace std;

template <typename TAirport>
constexpr auto construct_empty_airports() {
  
  constexpr auto num = static_cast<size_t>(TAirport::Last_);
  array<pair<TAirport, size_t>, num> res;
  for (int i = 0; i < num; ++i) {
    res[i].first = static_cast<TAirport>(i);
    res[i].second = 0;
  }
  return res;
}

// TAirport should be enum with sequential items and last item TAirport::Last_
template <typename TAirport>
class AirportCounter {
 public:
  AirportCounter() = default;

  template <typename TIterator>
  AirportCounter(TIterator begin, TIterator end) {
    for (auto it = begin; it < end; ++it) {
      ++airports_[static_cast<size_t>(*it)].second;
    }
  }

  size_t Get(TAirport airport) const {
    return airports_[static_cast<size_t>(airport)].second;
  }

  void Insert(TAirport airport) {
    auto idx = static_cast<size_t>(airport);
    ++airports_[idx].second;
  }

  void EraseOne(TAirport airport) {
    auto idx = static_cast<size_t>(airport);
    auto& flights = airports_[idx].second;
    if (flights > 0) {
      --flights;
    }
  }

  void EraseAll(TAirport airport) {
    auto idx = static_cast<size_t>(airport);
    airports_[idx].second = 0;
  }

  using Item = pair<TAirport, size_t>;
  using Items = array<Item, static_cast<size_t>(TAirport::Last_)>;

  Items const& GetItems() const { return airports_; }

 private:
  Items airports_ = construct_empty_airports<TAirport>();
};
