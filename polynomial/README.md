## Polynomial, operator[]

You are given with an implementation of class *Polynomial<T>* which represents a polynom of a single variable. Type *T* defines a type of both coefficients and variable. The interface is the following:
```
template<typename T>
class Polynomial {
private:
  ...
public:
  // Accepts a vector of coefficients (in ascending order)
  // For instance, Polynomial({10, 2, 3, 4}) would define a polynom 4*x^3 + 3*x^2 + 2*x + 10 
  Polynomial(vector<T> coeffs);

  // Create a polynom with 0 coefficients
  Polynomial();

  // The same as a Polynomial(vector<T>) but accepts a pair of iterators
  template<typename Iterator>
  Polynomial(Iterator first, Iterator last);

  // Equality operators. To polynoms are considered equal if and only if all coefficients of the same power are equal
  bool operator ==(const Polynomial& other) const;
  bool operator !=(const Polynomial& other) const;

  // Returns rank of a polynom.
  // For instance, rank of 2*x is 1, rank of 4*x^5 + x^3 - 10 is 5,
  // rank of a polynom 5 is 0.
  // Complexity is O(1)
  int Degree() const;

  Polynomial& operator +=(const Polynomial& r);
  Polynomial& operator -=(const Polynomial& r);

  // Returns a coefficient for a given degree.
  // Returns 0 if a degree is greater than polynom's rank.
  T operator [](size_t degree) const;
  // Computes a value of the polynom in a given point
  T operator ()(const T& x) const;

  using const_iterator = typename std::vector<T>::const_iterator;

  // Iterators on polynom's coefficients (ascending order)
  const_iterator begin() const;
  const_iterator end() const;
};
```

The goal of the task is to implement non-const version of *operator[]* which:
* would allow setting coefficient of a given degree
* would allow getting coefficient of a given degree
* would have amortized O(1) complexity
* would not change a rank of the polynom if no value is set 
