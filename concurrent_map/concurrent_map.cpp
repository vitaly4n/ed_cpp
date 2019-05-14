#include <map>
#include <mutex>
#include <utility>
#include <vector>

using namespace std;

template<typename K, typename V, typename Hash = std::hash<K>>
class ConcurrentMap
{
public:
  using MyType = unordered_map<K, V, Hash>;
  using protected_value_type = pair<MyType, mutable mutex>;

  class WriteAccess
  {
  public:
    WriteAccess(mutex& m, V& ref)
      : m_{ m }
      , ref_to_value{ ref }
    {}
    ~WriteAccess() { m_.unlock(); }

    WriteAccess(const WriteAccess& other) = delete;
    WriteAccess& operator=(const WriteAccess& other) = delete;

    V& ref_to_value;

  private:
    mutex& m_;
  };
  class ReadAccess
  {
  public:
    V& ref_to_value;
  };

  explicit ConcurrentMap(size_t bucket_count)
    : maps_{ bucket_count }
  {}

  WriteAccess operator[](const K& key)
  {
    auto& protected_map = get_by_key(key);

    protected_map.second.lock();
    auto& val = protected_map.first[key];
    return { protected_map.second, val };
  }

  MyType BuildOrdinaryMap() const
  {
    MyType res;
    for (auto& protected_map : maps_) {
      // lock_guard<mutex> lck(protected_map.second);
      res.insert(begin(protected_map.first), end(protected_map.first));
    }
    return res;
  }

  ReadAccess At(const K& key) const
  { // TODO
    static V val;
    return ReadAccess{ val };
  }

  bool Has(const K& key) const
  { // TODO
    return false;
  }

private:
  protected_value_type& get_by_key(const K& key)
  {
    const auto idx = Hash{}(key) % maps_.size();
    return maps_[idx];
  }

  vector<protected_value_type> maps_;
};
