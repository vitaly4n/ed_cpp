#include <utility>

// Исключение этого типа должно генерироваться при обращении к "пустому" Optional в функции Value
struct BadOptionalAccess
{};

template<typename T>
class Optional
{
private:
  // alignas нужен для правильного выравнивания блока памяти
  alignas(T) unsigned char data[sizeof(T)];
  bool defined = false;

  template<typename... V>
  void Construct(V&&... val)
  {
    defined = true;
    new (data) T(std::forward<V...>(val...));
  }

  template<typename V>
  void Assign(V&& val)
  {
    if (defined) {
      *Cast() = std::forward<V>(val);
    } else {
      Construct(std::forward<V>(val));
    }
  }

  T* Cast() {
    return reinterpret_cast<T*>(data);
  }

  const T* Cast() const {
    return reinterpret_cast<const T*>(data);
  }

public:
  Optional() = default;
  Optional(const T& elem);
  Optional(T&& elem);
  Optional(const Optional& other);
  Optional(Optional&& other);

  Optional& operator=(const T& elem);
  Optional& operator=(T&& elem);
  Optional& operator=(const Optional& other);
  Optional& operator=(Optional&& other);

  bool HasValue() const;

  // Эти операторы не должны делать никаких проверок на пустоту.
  // Проверки остаются на совести программиста.
  T& operator*();
  const T& operator*() const;
  T* operator->();
  const T* operator->() const;

  // Генерирует исключение BadOptionalAccess, если объекта нет
  T& Value();
  const T& Value() const;

  void Reset();

  ~Optional();
};

template<typename T>
Optional<T>::Optional(const T& elem)
{
  Construct(elem);
}

template<typename T>
Optional<T>::Optional(T&& elem)
{
  Construct(std::move(elem));
}

template<typename T>
Optional<T>::Optional(const Optional& other)
{
  if (other.defined) {
    Construct(*other);
  }
}

template<typename T>
Optional<T>::Optional(Optional&& other)
{
  if (other.defined) {
    Construct(std::move(*other));
  }
}

template<typename T>
Optional<T>&
Optional<T>::operator=(const T& elem)
{
  Assign(elem);
  return *this;
}

template<typename T>
Optional<T>&
Optional<T>::operator=(T&& elem)
{
  Assign(std::move(elem));
  return *this;
}

template<typename T>
Optional<T>&
Optional<T>::operator=(const Optional& other)
{
  if (other.defined) {
    Assign(*other);
  } else {
    Reset();
  }
  return *this;
}

template<typename T>
Optional<T>&
Optional<T>::operator=(Optional&& other)
{
  if (other.defined) {
    Assign(std::move(*other));
  } else {
    Reset();
  }
  return *this;
}

template<typename T>
bool Optional<T>::HasValue() const
{
  return defined;
}

template<typename T>
T& Optional<T>::operator*()
{
  return *Cast();
}

template<typename T>
const T& Optional<T>::operator*() const
{
  return *Cast();
}

template<typename T>
T* Optional<T>::operator->()
{
  return Cast();
}

template<typename T>
const T* Optional<T>::operator->() const
{
  return Cast();
}

template<typename T>
T& Optional<T>::Value()
{
  if (!defined) {
    throw BadOptionalAccess{};
  }
  return *Cast();
}

template<typename T>
const T& Optional<T>::Value() const
{
  if (!defined) {
    throw BadOptionalAccess{};
  }
  return *Cast();
}

template<typename T>
void
Optional<T>::Reset()
{
  if (defined) {
    Cast()->~T();
    defined = false;
  }
}

template<typename T>
Optional<T>::~Optional()
{
  Reset();
}
