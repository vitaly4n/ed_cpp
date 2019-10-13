#include <functional>
#include <memory>

template<typename T>
class LazyValue
{
public:
  using Builder = std::function<T()>;

  explicit LazyValue(Builder init)
    : builder_{ init }
  {}

  bool HasValue() const
  { 
    return !!obj_.get();
  }
  const T& Get() const
  {
    if (!obj_) {
      obj_.reset(new T{builder_()});
    }
    return *obj_;
  }

private:
  Builder builder_;
  mutable std::unique_ptr<T> obj_;
};
