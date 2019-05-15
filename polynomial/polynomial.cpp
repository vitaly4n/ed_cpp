#include <cstdlib>
#include <utility>
#include <vector>

using namespace std;

template<typename T>
class Polynomial
{
private:
  std::vector<T> coeffs_ = { 0 };

  void Shrink()
  {
    while (!coeffs_.empty() && coeffs_.back() == 0) {
      coeffs_.pop_back();
    }
  }

public:
  Polynomial() = default;
  Polynomial(std::vector<T> coeffs)
    : coeffs_(std::move(coeffs))
  {
    Shrink();
  }

  template<typename Iterator>
  Polynomial(Iterator first, Iterator last)
    : coeffs_(first, last)
  {
    Shrink();
  }

  bool operator==(const Polynomial& other) const
  {
    return coeffs_ == other.coeffs_;
  }

  bool operator!=(const Polynomial& other) const { return !operator==(other); }

  int Degree() const { return coeffs_.size() - 1; }

  Polynomial& operator+=(const Polynomial& r)
  {
    if (r.coeffs_.size() > coeffs_.size()) {
      coeffs_.resize(r.coeffs_.size());
    }
    for (std::size_t i = 0; i != r.coeffs_.size(); ++i) {
      coeffs_[i] += r.coeffs_[i];
    }
    Shrink();
    return *this;
  }

  Polynomial operator+(const Polynomial& r) const
  {
    Polynomial res = *this;
    res += r;
    return res;
  }

  Polynomial operator-(const Polynomial& r) const
  {
    Polynomial res = *this;
    res -= r;
    return res;
  }

  Polynomial& operator-=(const Polynomial& r)
  {
    if (r.coeffs_.size() > coeffs_.size()) {
      coeffs_.resize(r.coeffs_.size());
    }
    for (std::size_t i = 0; i != r.coeffs_.size(); ++i) {
      coeffs_[i] -= r.coeffs_[i];
    }
    Shrink();
    return *this;
  }

  T operator[](std::size_t degree) const
  {
    return degree < coeffs_.size() ? coeffs_[degree] : 0;
  }

  class CoeffAccess
  {
  public:
    CoeffAccess(const CoeffAccess&) = delete;
    CoeffAccess& operator=(const CoeffAccess&) = delete;

    CoeffAccess& operator=(T val)
    {
      if (polynom_coeffs_.size() <= degree_ && val != 0) {
        polynom_coeffs_.resize(degree_ + 1, 0);
      }
      if (degree_ < polynom_coeffs_.size()) {
        polynom_coeffs_[degree_] = move(val);
      }
      return *this;
    }

    operator T() const
    {
      return degree_ < polynom_coeffs_.size() ? polynom_coeffs_[degree_]
                                              : T{ 0 };
    }

  private:
    friend Polynomial;

    CoeffAccess(std::vector<T>& polynom_coeffs, std::size_t degree)
      : polynom_coeffs_{ polynom_coeffs }
      , degree_{ degree }
    {}
    CoeffAccess(CoeffAccess&&) = default;
    CoeffAccess& operator=(CoeffAccess&&) = default;

    std::vector<T>& polynom_coeffs_;
    std::size_t degree_ = 0;
  };

  CoeffAccess operator[](std::size_t degree)
  { // TODO
    return { coeffs_, degree };
  }

  T operator()(const T& x) const
  {
    T res = 0;
    for (auto it = coeffs_.rbegin(); it != coeffs_.rend(); ++it) {
      res *= x;
      res += *it;
    }
    return res;
  }

  using const_iterator = typename std::vector<T>::const_iterator;

  const_iterator begin() const { return coeffs_.cbegin(); }

  const_iterator end() const { return coeffs_.cend(); }
};
