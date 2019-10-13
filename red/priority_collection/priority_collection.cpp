#include <list>
#include <set>
#include <utility>
#include <vector>

using namespace std;

template <typename It>
struct IteratorPriorityComparator {
  bool operator()(It const& lhs, It const& rhs) const {
    if (lhs->second < rhs->second) {
      return true;
    } else if (lhs->second > rhs->second) {
      return false;
    } else {
      return lhs < rhs;
    }
  }
};

template <typename T>
class PriorityCollection {
 public:
  using PriorityObject = pair<T, int>;
  using PriorityObjectArray = vector<PriorityObject>;
  using PriorityObjectIterator = typename PriorityObjectArray::iterator;

  using PriorityIteratorSet =
      set<PriorityObjectIterator,
          IteratorPriorityComparator<PriorityObjectIterator>>;

  using Id = PriorityObjectIterator;

  PriorityCollection() { objects_.reserve(1000001); }

  Id Add(T object) {
    Id obj_it = objects_.insert(end(objects_), {move(object), 0});
    priorities_.insert(obj_it);
    return obj_it;
  }

  template <typename ObjInputIt, typename IdOutputIt>
  void Add(ObjInputIt range_begin, ObjInputIt range_end, IdOutputIt ids_begin) {
    for (; range_begin != range_end; ++range_begin, ++ids_begin) {
      *ids_begin = Add(move(*range_begin));
    }
  }

  bool IsValid(Id id) const { 
    if (id >= begin(objects_) && id < end(objects_)) {
      return id->second >= 0;
    }
    return false;
  }
  const T& Get(Id id) const { return id->first; }

  void Promote(Id id) {
    auto const priority_it = priorities_.find(id);
    priorities_.erase(priority_it);

    ++id->second;
    priorities_.insert(id);
  }

  pair<const T&, int> GetMax() const {
    auto it = rbegin(priorities_);
    return {(*it)->first, (*it)->second};
  }

  pair<T, int> PopMax() {
    auto it = --end(priorities_);
    auto& obj = *(*it);
    
    pair<T, int> res;
    res.first = move(obj.first);
    res.second = obj.second;
    obj.second = -1;
    
    priorities_.erase(it);
    return res;
  }

 private:
  PriorityObjectArray objects_;
  PriorityIteratorSet priorities_;
};
