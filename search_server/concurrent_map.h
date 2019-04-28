#pragma once

#include <map>
#include <mutex>
#include <utility>
#include <vector>
#include <optional>

using namespace std;

const size_t THREAD_COUNT = 8;

template <typename K, typename V>
class ConcurrentMap {
 public:
  using value_type = map<K, V>;
  using protected_value_type = pair<value_type, mutex>;

  class Access {
   public:
    Access(mutex& m, V& ref) : m_{m}, ref_to_value{ref} { }
    ~Access() { m_.unlock(); }

    Access(const Access& other) = delete;
    Access& operator=(const Access& other) = delete;

    V& ref_to_value;

   private:
    mutex& m_;
  };

  explicit ConcurrentMap(size_t bucket_count) : maps_{bucket_count} {}

  Access operator[](const K& key) {
    auto& protected_map = get_by_key(key);

    protected_map.second.lock();
    auto& val = protected_map.first[key];
    return {protected_map.second, val};
  }

  optional<Access> find(const K& key) {
    auto& protected_map = get_by_key(key);

    protected_map.second.lock();
    auto it = protected_map.first.find(key);
    if (it != protected_map.first.end()) {
      return optional<Access>{in_place, protected_map.second, it->second};
    } else {
      protected_map.second.unlock();
      return {};
    }
  }

  map<K, V> BuildOrdinaryMap() {
    map<K, V> res;
    for (auto& protected_map : maps_) {
      lock_guard<mutex> lck(protected_map.second);
      res.insert(begin(protected_map.first), end(protected_map.first));
    }
    return res;
  }

 private:
  protected_value_type& get_by_key(const K& key) {
    auto const hashval = hash<K>()(key);
    return maps_[hashval % maps_.size()];
  }

  vector<protected_value_type> maps_;
};
