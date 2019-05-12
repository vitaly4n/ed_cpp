#include <functional>

template<typename T>
class LazyValue
{
public:
  explicit LazyValue(std::function<T()> init) {}

  bool HasValue() const
  { // TODO
    return false;
  }
  const T& Get() const
  { // TODO
    static T t;
    return t;
  }

private:
};
