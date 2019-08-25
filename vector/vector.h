#include <cstddef>
#include <memory>
#include <utility>

template<typename T>
struct Buffer
{
  T* data = nullptr;
  size_t capacity = 0;

  static T* Allocate(size_t size) { return static_cast<T*>(operator new(size * sizeof(T))); }
  static void Deallocate(T* data) { operator delete(data); }

  Buffer() = default;
  Buffer(size_t size)
    : data(Allocate(size))
    , capacity(size)
  {}

  Buffer(const Buffer&) = delete;
  Buffer(Buffer&& other) noexcept { Swap(other); }

  Buffer& operator=(const Buffer&) = delete;
  Buffer& operator=(Buffer&& other) noexcept
  {
    Swap(other);
    return *this;
  }

  ~Buffer() { Deallocate(data); }

  void Swap(Buffer& other)
  {
    std::swap(data, other.data);
    std::swap(capacity, other.capacity);
  }

  T& operator[](size_t i) { return *(data + i); }
  const T& operator[](size_t i) const { return *(data + i); }

  T& operator*() { return *data; }
  const T& operator*() const { return *data; }
};

template<typename T>
class Vector
{
public:
  Vector() = default;
  Vector(size_t n);
  Vector(const Vector& other);
  Vector(Vector&& other);

  ~Vector();

  Vector& operator=(const Vector& other);
  Vector& operator=(Vector&& other) noexcept;

  void Reserve(size_t n);

  void Resize(size_t n);

  void PushBack(const T& elem);
  void PushBack(T&& elem);

  template<typename... Args>
  T& EmplaceBack(Args&&... args)
  {
    if (size_ == buf_.capacity) {
      Reserve(size_ == 0 ? 1 : size_ * 2);
    }
    T* res = new (buf_.data + size_) T(std::forward<Args>(args)...);
    ++size_;
    return *res;
  }

  void PopBack();

  size_t Size() const noexcept;

  size_t Capacity() const noexcept;

  const T& operator[](size_t i) const;
  T& operator[](size_t i);

  using iterator = T*;
  using const_iterator = const T*;

  iterator begin() noexcept;
  iterator end() noexcept;

  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;

  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;

  iterator Insert(const_iterator pos, const T& elem);
  iterator Insert(const_iterator pos, T&& elem);

  template<typename... Args>
  iterator Emplace(const_iterator it, Args&&... args)
  {
    const size_t idx = it - begin();
    if (size_ == buf_.capacity) {
      Reserve(size_ == 0 ? 1 : size_ * 2);
    }

    T elt_to_add(std::forward<Args>(args)...);

    Buffer<T> tmp(size_ - idx);
    std::uninitialized_move_n(buf_.data + idx, size_ - idx, tmp.data);
    new (buf_.data + idx) T(std::move(elt_to_add));
    std::uninitialized_move_n(tmp.data, size_ - idx, buf_.data + idx + 1);
    ++size_;
    return buf_.data + idx;
  }

  iterator Erase(const_iterator it);

private:
  void Swap(Vector& other)
  {
    buf_.Swap(other.buf_);
    std::swap(size_, other.size_);
  }

  Buffer<T> buf_;
  size_t size_ = 0;
};

template<typename T>
Vector<T>::Vector(size_t n)
  : buf_(n)
  , size_(n)
{
  std::uninitialized_value_construct_n(buf_.data, size_);
}

template<typename T>
Vector<T>::Vector(const Vector& other)
  : buf_(other.size_)
  , size_(other.size_)
{
  std::uninitialized_copy_n(other.buf_.data, size_, buf_.data);
}

template<typename T>
Vector<T>::Vector(Vector&& other)
{
  Swap(other);
}

template<typename T>
Vector<T>::~Vector()
{
  std::destroy_n(buf_.data, size_);
}

template<typename T>
Vector<T>&
Vector<T>::operator=(const Vector& other)
{
  if (other.size_ > buf_.capacity) {
    Vector tmp(other);
    Swap(tmp);
  } else {
    for (size_t i = 0; i < size_ && i < other.size_; ++i) {
      buf_[i] = other.buf_[i];
    }
    if (size_ > other.size_) {
      std::destroy_n(buf_.data + other.size_, size_ - other.size_);
    } else if (size_ < other.size_) {
      std::uninitialized_copy_n(other.buf_.data + other.size_, other.size_ - size_, buf_.data + size_);
    }
  }
  size_ = other.size_;
  return *this;
}

template<typename T>
Vector<T>&
Vector<T>::operator=(Vector&& other) noexcept
{
  Swap(other);
  return *this;
}

template<typename T>
void
Vector<T>::Reserve(size_t n)
{
  if (n > buf_.capacity) {
    Buffer<T> reserved(n);
    std::uninitialized_move_n(buf_.data, size_, reserved.data);
    std::destroy_n(buf_.data, size_);
    buf_.Swap(reserved);
  }
}

template<typename T>
void
Vector<T>::Resize(size_t n)
{
  Reserve(n);
  if (size_ > n) {
    std::destroy_n(buf_.data + n, size_ - n);
  } else if (size_ < n) {
    std::uninitialized_value_construct_n(buf_.data + size_, n - size_);
  }
  size_ = n;
}

template<typename T>
void
Vector<T>::PushBack(const T& elem)
{
  EmplaceBack(elem);
}

template<typename T>
void
Vector<T>::PushBack(T&& elem)
{
  EmplaceBack(std::move(elem));
}

template<typename T>
void
Vector<T>::PopBack()
{
  std::destroy_at(buf_.data + size_ - 1);
  --size_;
}

template<typename T>
size_t
Vector<T>::Size() const noexcept
{
  return size_;
}

template<typename T>
size_t
Vector<T>::Capacity() const noexcept
{
  return buf_.capacity;
}

template<typename T>
const T& Vector<T>::operator[](size_t i) const
{
  return buf_[i];
}

template<typename T>
T& Vector<T>::operator[](size_t i)
{
  return buf_[i];
}

template<typename T>
typename Vector<T>::iterator
Vector<T>::begin() noexcept
{
  return buf_.data;
}

template<typename T>
typename Vector<T>::iterator
Vector<T>::end() noexcept
{
  return buf_.data + size_;
}

template<typename T>
typename Vector<T>::const_iterator
Vector<T>::begin() const noexcept
{
  return buf_.data;
}

template<typename T>
typename Vector<T>::const_iterator
Vector<T>::end() const noexcept
{
  return buf_.data + size_;
}
template<typename T>
typename Vector<T>::const_iterator
Vector<T>::cbegin() const noexcept
{
  return buf_.data;
}

template<typename T>
typename Vector<T>::const_iterator
Vector<T>::cend() const noexcept
{
  return buf_.data + size_;
}

template<typename T>
typename Vector<T>::iterator
Vector<T>::Insert(typename Vector<T>::const_iterator pos, const T& elem)
{
  return Emplace(pos, elem);
}

template<typename T>
typename Vector<T>::iterator
Vector<T>::Insert(typename Vector<T>::const_iterator pos, T&& elem)
{
  return Emplace(pos, std::move(elem));
}

template<typename T>
typename Vector<T>::iterator
Vector<T>::Erase(typename Vector<T>::const_iterator it)
{
  if (it == end()) {
    return end();
  }

  std::destroy_at(it);

  const size_t idx = it - begin();
  const size_t tmp_buf_size = end() - it - 1;
  Buffer<T> tmp(tmp_buf_size);
  std::uninitialized_move_n(buf_.data + idx + 1, tmp_buf_size, tmp.data);
  std::uninitialized_move_n(tmp.data, tmp_buf_size, buf_.data + idx);
  --size_;
  return buf_.data + idx;
}
