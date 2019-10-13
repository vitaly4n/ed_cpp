#pragma once

#include <cstdlib>

#include <memory>

using namespace std;

template <typename T>
class SimpleVector {
 public:
  SimpleVector() {}
  explicit SimpleVector(size_t size)
      : pool_{make_unique<T[]>(size)}, size_{size}, capacity_{size} {}

  T& operator[](size_t index) { return *(pool_.get() + index); }

  T* begin() { return size_ == 0 ? nullptr : pool_.get(); }
  T* end() { return size_ == 0 ? nullptr : pool_.get() + Size(); };

  size_t Size() const { return size_; }
  size_t Capacity() const { return capacity_; }
  void PushBack(const T& value) {
    if (capacity_ <= size_) {
      auto new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
      auto pool2 = make_unique<T[]>(new_capacity);
      if (pool_.get()) {
        move(pool_.get(), pool_.get() + size_, pool2.get());
      }
      pool_ = move(pool2);
      capacity_ = new_capacity;
    }
    *(pool_.get() + size_) = value;
    ++size_;
  }

 private:
  size_t capacity_ = 0;
  size_t size_ = 0;

  unique_ptr<T[]> pool_;
};
