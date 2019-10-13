#include <cstddef>
#include <utility>

// to guarantee compilation on the server
using namespace std;

template<typename T>
class UniquePtr
{
private:
  T* data_ = nullptr;

private:
  void Delete()
  {
    if (data_) {
      delete data_;
      data_ = nullptr;
    }
  }

public:
  UniquePtr();
  UniquePtr(T* ptr);
  UniquePtr(const UniquePtr&) = delete;
  UniquePtr(UniquePtr&& other);
  UniquePtr& operator=(const UniquePtr&) = delete;
  UniquePtr& operator=(nullptr_t);
  UniquePtr& operator=(UniquePtr&& other);
  ~UniquePtr();

  T& operator*() const;

  T* operator->() const;

  T* Release();

  void Reset(T* ptr);

  void Swap(UniquePtr& other);

  T* Get() const;
};

template<typename T>
UniquePtr<T>::UniquePtr() = default;

template<typename T>
UniquePtr<T>::UniquePtr(T* ptr)
{
  data_ = ptr;
}

template<typename T>
UniquePtr<T>::UniquePtr(UniquePtr<T>&& ptr)
{
  data_ = ptr.data_;
  ptr.data_ = nullptr;
}

template<typename T>
UniquePtr<T>&
UniquePtr<T>::operator=(UniquePtr<T>&& ptr)
{
  Delete();
  data_ = ptr.data_;
  ptr.data_ = nullptr;
  return *this;
}

template<typename T>
UniquePtr<T>& UniquePtr<T>::operator=(nullptr_t)
{
  Delete();
  return *this;
}

template<typename T>
UniquePtr<T>::~UniquePtr()
{
  Delete();
}

template<typename T>
T& UniquePtr<T>::operator*() const
{
  return *data_;
}

template<typename T>
T* UniquePtr<T>::operator->() const
{
  return data_;
}

template<typename T>
T*
UniquePtr<T>::Release()
{
  auto res = data_;
  data_ = nullptr;
  return res;
}

template<typename T>
void
UniquePtr<T>::Reset(T* ptr)
{
  Delete();
  data_ = ptr;
}

template<typename T>
void
UniquePtr<T>::Swap(UniquePtr<T>& other)
{
  swap(data_, other.data_);
}

template<typename T>
T*
UniquePtr<T>::Get() const
{
  return data_;
}
