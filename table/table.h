#pragma once

#include "common.h"

template<typename T>
class Table
{
public:
  Table() = default;
  Table(const Size& size)
    : table_(size.rows, RowData(size.cols))
    , size_(size)
  {}

  Size GetSize() const { return size_; }
  bool IsInside(const Size& size) const { return size.rows <= size_.rows && size.cols <= size_.cols; }
  bool IsInside(const Position& pos) const { return pos.row < size_.rows && pos.col < size_.cols; }

  void Grow(const Size& new_size)
  {
    if (new_size.rows > size_.rows) {
      table_.resize(new_size.rows, RowData(size_.cols));
      size_.rows = new_size.rows;
    }
    if (new_size.cols > size_.cols) {
      for (auto& row : table_) {
        row.resize(new_size.cols);
      }
      size_.cols = new_size.cols;
    }
  }

  void InsertRows(int before, int count = 1)
  {
    if (size_.rows <= before || size_.rows + count > Position::kMaxRows) {
      throw TableTooBigException("Rows limit exceeded");
    }

    TableData subtable(count, RowData(size_.cols));
    table_.insert(table_.begin() + before, make_move_iterator(begin(subtable)), make_move_iterator(end(subtable)));
    size_.rows += count;
  }
  void InsertCols(int before, int count = 1)
  {
    if (size_.cols <= before || size_.cols + count > Position::kMaxCols) {
      throw TableTooBigException("Columns limit exceeded");
    }

    RowData subrow(count);
    for (auto& row : table_) {
      row.insert(row.begin() + before, begin(subrow), end(subrow));
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
  }
  void DeleteCols(int first, int count = 1)
  {
    if (first + count >= size_.cols) {
      throw std::out_of_range("Cannot delete columns outside of printable area");
    }

    for (auto& row : table_) {
      row.erase(begin(row) + first, begin(row) + first + count);
    }
    size_.cols -= count;
  }

  T& operator()(size_t row, size_t col) { return table_[row][col]; }
  const T& operator()(size_t row, size_t col) const { return table_[row][col]; }
  T& operator()(const Position& pos) { return operator()(pos.row, pos.col); }
  const T& operator()(const Position& pos) const { return operator()(pos.row, pos.col); }

  template<typename F>
  void ForEach(F func) const
  {
    for (int i = 0; i < size_.rows; ++i) {
      for (int j = 0; j < size_.cols; ++j) {
        func(operator()(i, j));
      }
    }
  }

  template<typename F>
  void ForEach(F func)
  {
    for (int i = 0; i < size_.rows; ++i) {
      for (int j = 0; j < size_.cols; ++j) {
        func(i, j, operator()(i, j));
      }
    }
  }

private:
  using RowData = std::vector<T>;
  using TableData = std::vector<RowData>;

  std::vector<std::vector<T>> table_;
  Size size_;
};
