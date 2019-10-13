#include <memory>
#include <queue>
#include <map>

using namespace std;

template <class T>
class ObjectPool {
 public:
  T* Allocate() {
    if (pool_deallocated_.empty()) {
      auto obj = make_unique<T>();
      auto ptr = obj.get();
      return pool_allocated_.emplace(ptr, std::move(obj)).first->first;
    }
    return TryAllocate();
  }

  T* TryAllocate() {
    if (!pool_deallocated_.empty()) {
      auto obj = std::move(pool_deallocated_.front());
      auto ptr = obj.get();
      pool_deallocated_.pop(); 
      return pool_allocated_.emplace(ptr, std::move(obj)).first->first;
    }
    return nullptr;
  }

  void Deallocate(T* object) {
    auto alloc_it = pool_allocated_.find(object);
    if (alloc_it == end(pool_allocated_)) {
      throw invalid_argument("The object is not allocated");
    }

    pool_deallocated_.push(std::move(alloc_it->second));
    pool_allocated_.erase(alloc_it);
  }

 private:
  using TPtr = unique_ptr<T>;

  queue<TPtr> pool_deallocated_;
  map<T*, TPtr> pool_allocated_;
};
