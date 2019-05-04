#pragma once

#include <mutex>
#include <shared_mutex>
#include <utility>

template <typename T>
struct Synchronized {
 public:
  struct Access {
    Access(T& ref, std::unique_lock<std::shared_mutex>&& lock)
        : ref_{ref}, lock_{std::move(lock)} {}

    T& ref_;

   private:
    std::unique_lock<std::shared_mutex> lock_;
  };
  struct ConstAccess {
    ConstAccess(T const& ref, std::shared_lock<std::shared_mutex>&& lock)
        : ref_{ref}, lock_{std::move(lock)} {}

    T const& ref_;

   private:
    std::shared_lock<std::shared_mutex> lock_;
  };

  Synchronized() = default;
  Synchronized(T const& obj) : obj_{obj} {}
  Synchronized(T&& obj) : obj_{std::move(obj)} {}

  Access get() {
    std::unique_lock<std::shared_mutex> lock(m_);
    return Access(obj_, std::move(lock));
  }

  ConstAccess get() const {
    std::shared_lock<std::shared_mutex> lock(m_);
    return ConstAccess(obj_, std::move(lock));
  }

 private:
  mutable std::shared_mutex m_;
  T obj_;
};
