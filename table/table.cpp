#include <cstddef>
#include <vector>

using namespace std;

template <typename T>
class Table {
 public:
  Table(size_t rows, size_t cols)
      : rows_{rows}, cols_{cols}, vals_{rows, vector<T>(cols, T{})} {}

  vector<T>& operator[](size_t idx) { return vals_[idx]; }
  vector<T> const& operator[](size_t idx) const { return vals_[idx]; }

  void Resize(size_t rows, size_t cols) {
    vals_.resize(rows, vector<T>(cols, T{}));

    for (auto& row_val : vals_) {
      if (cols != row_val.size()) {
        row_val.resize(cols);
      }
    }

    rows_ = rows;
    cols_ = cols;
  }

  pair<size_t, size_t> Size() const { return {rows_, cols_}; }

 private:
  size_t rows_ = 0;
  size_t cols_ = 0;
  vector<vector<T>> vals_;
};
