#pragma once

#include "common.h"

#include <optional>
#include <unordered_map>

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

struct PositionHasher
{
public:
  size_t operator()(const Position& pos) const { return (pos.row << 14) + pos.col; }
};

template<typename T>
class Table2
{
public:
  Size GetSize() const { return size_; }
  bool IsInside(const Size& size) const { return size.rows <= size_.rows && size.cols <= size_.cols; }
  bool IsInside(const Position& pos) const { return pos.row < size_.rows && pos.col < size_.cols; }

  void Grow(const Size& new_size)
  {
    size_.rows = std::max(new_size.rows, size_.rows);
    size_.cols = std::max(new_size.cols, size_.cols);
  }

  const T* GetAt(int row, int col) const { return GetAt(Position{ row, col }); }
  const T* GetAt(const Position& pos) const
  {
    if (!IsInside(pos)) {
      return nullptr;
    }

    auto it = table_.find(pos);
    return it != table_.end() ? &it->second : nullptr;
  }

  T& operator()(int row, int col) { return operator()(Position{ row, col }); }
  T& operator()(const Position& pos)
  {
    if (!IsInside(pos)) {
      Grow(Size{ pos.row + 1, pos.col + 1 });
    }
    return table_[pos];
  }

  void InsertRows(int before, int count = 1)
  {
    if (size_.rows <= before || size_.rows + count > Position::kMaxRows) {
      throw TableTooBigException("Rows limit exceeded");
    }

    std::vector<std::pair<Position, T>> shelter;
    for (int row = before; row < size_.rows; ++row) {
      for (int col = 0; col < size_.cols; ++col) {
        const Position pos{row, col};
        auto it = table_.find(pos);
        if (it != table_.end()) {
          shelter.emplace_back(pos, std::move(it->second));
          table_.erase(it);
        }
      }
    }

    for (auto& [pos, val] : shelter) {
      table_.emplace(Position{pos.row + count, pos.col}, std::move(val));
    }

    size_.rows += count;
  }

  void InsertCols(int before, int count = 1)
  {
    if (size_.cols <= before || size_.cols + count > Position::kMaxCols) {
      throw TableTooBigException("Columns limit exceeded");
    }

    std::vector<std::pair<Position, T>> shelter;
    for (int row = 0; row < size_.rows; ++row) {
      for (int col = before; col < size_.cols; ++col) {
        const Position pos{row, col};
        auto it = table_.find(pos);
        if (it != table_.end()) {
          shelter.emplace_back(pos, std::move(it->second));
          table_.erase(it);
        }
      }
    }

    for (auto& [pos, val] : shelter) {
      table_.emplace(Position{pos.row, pos.col + count}, std::move(val));
    }

    size_.cols += count;
  }

  void DeleteRows(int first, int count = 1)
  {
    if (first + count >= size_.rows) {
      throw std::out_of_range("Cannot delete rows outside of printable area");
    }

    for (int col = 0; col < size_.cols; ++col) {
      for (int row = first; row < first + count; ++row) {
        auto it = table_.find(Position{row, col});
        if (it != table_.end()) {
          table_.erase(it);
        }
      }
      for (auto row = first + count; row < size_.rows; ++row) {
        auto it = table_.find(Position{row, col});
        if (it != table_.end()) {
          table_.emplace(Position{row - count, col}, std::move(it->second));
          table_.erase(it);
        }
      }
    }

    size_.rows -= count;
    size_.cols *= size_.rows != 0;
  }

  void DeleteCols(int first, int count = 1)
  {
    if (first + count >= size_.cols) {
      throw std::out_of_range("Cannot delete columns outside of printable area");
    }

    for (int row = 0; row < size_.rows; ++row) {
      for (int col = first; col < first + count; ++col) {
        auto it = table_.find(Position{row, col});
        if (it != table_.end()) {
          table_.erase(it);
        }
      }
      for (int col = first + count; col < size_.cols; ++col) {
        auto it = table_.find(Position{row, col});
        if (it != table_.end()) {
          table_.emplace(Position{row, col - count}, std::move(it->second));
          table_.erase(it);
        }
      }
    }

    size_.cols -= count;
    size_.rows *= size_.cols != 0;
  }

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
  Size size_;
  std::unordered_map<Position, T, PositionHasher> table_;
};
