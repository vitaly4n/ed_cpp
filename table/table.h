#pragma once

#include "common.h"

#include <optional>

template<typename T>
class Table
{
public:
  Table() = default;
  Table(const Size& size)
    : table_(size.rows)
    , size_(size)
  {}

  Size GetSize() const { return size_; }
  bool IsInside(const Size& size) const { return size.rows <= size_.rows && size.cols <= size_.cols; }
  bool IsInside(const Position& pos) const { return pos.row < size_.rows && pos.col < size_.cols; }

  void Grow(const Size& new_size)
  {
    size_.rows = std::max(new_size.rows, size_.rows);
    size_.cols = std::max(new_size.cols, size_.cols);
    for (auto& row : table_) {
      if (row) {
        row->resize(size_.cols);
      }
    }
    table_.resize(size_.rows);
  }

  void InsertRows(int before, int count = 1)
  {
    if (size_.rows <= before || size_.rows + count > Position::kMaxRows) {
      throw TableTooBigException("Rows limit exceeded");
    }

    table_.insert(table_.begin() + before, count, RowData());
    size_.rows += count;
  }
  void InsertCols(int before, int count = 1)
  {
    if (size_.cols <= before || size_.cols + count > Position::kMaxCols) {
      throw TableTooBigException("Columns limit exceeded");
    }

    std::vector<T> subrow(count);
    for (auto& row : table_) {
      if (row) {
        row->insert(begin(*row) + before, make_move_iterator(begin(subrow)), make_move_iterator(end(subrow)));
      }
    }
    size_.cols += count;
  }
  void DeleteRows(int first, int count = 1)
  {
    if (first + count >= size_.rows) {
      throw std::out_of_range("Cannot delete rows outside of printable area");
    }

    table_.erase(begin(table_) + first, begin(table_) + first + count);
    size_.rows -= count;
    size_.cols *= size_.rows != 0;
  }
  void DeleteCols(int first, int count = 1)
  {
    if (first + count >= size_.cols) {
      throw std::out_of_range("Cannot delete columns outside of printable area");
    }

    for (auto& row : table_) {
      if (row) {
        row->erase(begin(*row) + first, begin(*row) + first + count);
      }
    }
    size_.cols -= count;
    size_.rows *= size_.cols != 0;
  }

  const T* GetAt(size_t row, size_t col) const { return table_[row] ? &(*table_[row])[col] : nullptr; }
  const T* GetAt(const Position& pos) const { return GetAt(pos.row, pos.col); }

  T& operator()(const size_t row, size_t col)
  {
    if (!table_[row]) {
      table_[row] = RowData(size_.cols);
    }
    return (*table_[row])[col];
  }
  T& operator()(const Position& pos) { return operator()(pos.row, pos.col); }

  template<typename F>
  void ForEach(F func) const
  {
    for (int i = 0; i < size_.rows; ++i) {
      for (int j = 0; j < size_.cols; ++j) {
        func(i, j, GetAt(i, j));
      }
    }
  }

  template<typename F>
  void ForEach(F func)
  {
    for (int i = 0; i < size_.rows; ++i) {
      for (int j = 0; j < size_.cols; ++j) {
        func(i, j, GetAt(i, j));
      }
    }
  }

private:
  using RowData = std::optional<std::vector<T>>;
  using TableData = std::vector<RowData>;

  TableData table_;
  Size size_;
};
