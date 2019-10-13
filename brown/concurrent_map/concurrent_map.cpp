#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

template<typename K, typename V, typename Hash = std::hash<K>>
class ConcurrentMap
{
public:
  using MyType = unordered_map<K, V, Hash>;

  struct ProtectedMap
  {
    MyType map;
    mutable shared_mutex mutex;
  };

  class WriteAccess
  {
  public:
    WriteAccess(unique_lock<shared_mutex>&& lock, V& ref)
      : lock_{ move(lock) }
      , ref_to_value{ ref }
    {}

    V& ref_to_value;

  private:
    unique_lock<shared_mutex> lock_;
  };
  class ReadAccess
  {
  public:
    ReadAccess(shared_lock<shared_mutex> lock, const V& ref)
      : lock_{ move(lock) }
      , ref_to_value{ ref }
    {}

    const V& ref_to_value;

  private:
    shared_lock<shared_mutex> lock_;
  };

  explicit ConcurrentMap(size_t bucket_count)
    : maps_{ bucket_count }
  {}

  WriteAccess operator[](const K& key)
  {
    auto& protected_map = get_by_key(key);

    unique_lock lock{ protected_map.mutex };
    auto& val = protected_map.map[key];
    return { move(lock), val };
  }

  ReadAccess At(const K& key) const
  {
    const auto& protected_map = get_by_key(key);

    shared_lock lock{ protected_map.mutex };
    const auto& val = protected_map.map.at(key);
    return { move(lock), val };
  }

  bool Has(const K& key) const
  {
    const auto& protected_map = get_by_key(key);

    shared_lock lock{ protected_map.mutex };
    return protected_map.map.count(key) > 0;
  }

  MyType BuildOrdinaryMap() const
  {
    MyType res;
    for (auto& protected_map : maps_) {
      shared_lock lock{ protected_map.mutex };
      res.insert(begin(protected_map.map), end(protected_map.map));
    }
    return res;
  }

private:
  ProtectedMap& get_by_key(const K& key)
  {
    const auto idx = Hash{}(key) % maps_.size();
    return maps_[idx];
  }
  const ProtectedMap& get_by_key(const K& key) const
  {
    const auto idx = Hash{}(key) % maps_.size();
    return maps_[idx];
  }

  vector<ProtectedMap> maps_;
};
