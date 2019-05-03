#pragma once

#include <map>
#include <optional>
#include <shared_mutex>
#include <utility>
#include <vector>

using namespace std;

const size_t THREAD_COUNT = 8;

template <typename K, typename V>
class ConcurrentMap {
 public:
  using map_type = map<K, V>;

  struct protected_map_type {
    map_type map_;
    mutable shared_mutex mutex_;
  };

  class Access {
   public:
    Access(unique_lock<shared_mutex>& lock, V& ref)
        : lock_{lock}, ref_to_value{ref} {}
    Access(const Access& other) = delete;
    Access& operator=(const Access& other) = delete;

    V& ref_to_value;

   private:
    unique_lock<shared_mutex>& lock_;
  };

  class ConstAccess {
   public:
    ConstAccess(shared_lock<shared_mutex>& lock, V const& ref)
        : lock_{lock}, ref_to_value{ref} {}
    ConstAccess(const ConstAccess& other) = delete;
    ConstAccess& operator=(const ConstAccess& other) = delete;

    V const& ref_to_value;

   private:
    shared_lock<shared_mutex>& lock_;
  };

  ConcurrentMap(size_t bucket_count = 7) : maps_{bucket_count} {}

  Access operator[](const K& key) {
    auto& protected_map = get_by_key(key);

    unique_lock lock{protected_map.mutex_};
    auto& val = protected_map.map_[key];
    return {lock, val};
  }

  optional<ConstAccess> find(const K& key) {
    auto& protected_map = get_by_key(key);

    shared_lock lock{protected_map.mutex_};
    auto it = protected_map.map_.find(key);

    if (it == protected_map.map_.end()) {
      return optional<ConstAccess>{};
    }
    return optional<ConstAccess>{in_place, lock, it->second};
  }

  map<K, V> BuildOrdinaryMap() {
    map<K, V> res;
    for (auto& protected_map : maps_) {
      lock_guard<shared_mutex> lck(protected_map.mutex_);
      res.insert(begin(protected_map.map_), end(protected_map.map_));
    }
    return res;
  }

 private:
  protected_map_type& get_by_key(const K& key) {
    auto const hashval = hash<K>()(key);
    return maps_[hashval % maps_.size()];
  }

  vector<protected_map_type> maps_;
};
