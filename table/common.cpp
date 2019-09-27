#include "common.h"
#include "formula.h"
#include "table.h"

#include <algorithm>
#include <charconv>
#include <deque>
#include <iostream>
#include <list>
#include <set>
#include <sstream>
#include <utility>

#include <thread>
#include <chrono>

using namespace std;

bool
Position::operator==(const Position& other) const
{
  return tie(row, col) == tie(other.row, other.col);
}

bool
Position::operator<(const Position& other) const
{
  return tie(row, col) < tie(other.row, other.col);
}

bool
Position::IsValid() const
{
  return row >= 0 && col >= 0 && row < kMaxRows && col < kMaxCols;
}

namespace {

string
IndexToExcelAlpha(int idx)
{
  string res;

  int num = idx + 1;
  while (num) {
    int rem = num % 26;
    if (rem == 0) {
      res += 'Z';
      num = (num - 1) / 26;
    } else {
      res += char('A' + rem - 1);
      num = num / 26;
    }
  }
  reverse(begin(res), end(res));

  return res;
}

optional<int>
ExcelAlphaToIndex(string_view& str, int max)
{
  const size_t init_size = str.size();

  int res = 0;
  while (!str.empty()) {
    const char cur_digit = str.front() - 'A' + 1;
    if (cur_digit <= 26 && cur_digit > 0) {
      if (max / 26 < res) {
        return nullopt;
      }
      res *= 26;
      if (max - cur_digit < res) {
        return nullopt;
      }
      res += cur_digit;
    } else {
      break;
    }
    str.remove_prefix(1);
  }

  return init_size != str.size() ? optional<int>(res) : nullopt;
}

optional<int>
NumStringToIndex(string_view& str, int max)
{
  const size_t init_size = str.size();

  int res = 0;
  while (!str.empty()) {
    const char cur_digit = str.front() - '0';
    if (cur_digit < 10 && cur_digit >= 0) {
      if (max / 10 < res) {
        return nullopt;
      }
      res *= 10;
      if (max - cur_digit < res) {
        return nullopt;
      }
      res += cur_digit;
    } else {
      break;
    }
    str.remove_prefix(1);
  }

  return init_size != str.size() ? optional<int>(res) : nullopt;
}

variant<string, double>
ParseValue(string_view str)
{
  if (str.empty()) {
    return 0.;
  }

  stringstream ss;
  ss << str;
  double val;
  if ((ss >> val) && ss.eof()) {
    return val;
  }

  auto escape_idx = str.find(kEscapeSign);
  if (escape_idx == 0) {
    str.remove_prefix(1);
  }
  return string(str);
}

} // namespace

string
Position::ToString() const
{
  if (!IsValid())
    return {};

  return IndexToExcelAlpha(col) + to_string(row + 1);
}

Position
Position::FromString(string_view str)
{
  int col_num = 0;
  int row_num = 0;
  if (auto col_res = ExcelAlphaToIndex(str, kMaxCols)) {
    col_num = *col_res;
  }
  if (auto row_res = NumStringToIndex(str, kMaxRows)) {
    row_num = *row_res;
  }
  return row_num && col_num && str.empty() ? Position{ row_num - 1, col_num - 1 } : Position{ -1, -1 };
}

bool
Size::operator==(const Size& other) const
{
  return tie(rows, cols) == tie(other.rows, other.cols);
}

FormulaError::FormulaError(FormulaError::Category category)
  : category_(category)
{}

FormulaError::Category
FormulaError::GetCategory() const
{
  return category_;
}

bool
FormulaError::operator==(FormulaError other) const
{
  return category_ == other.category_;
}

string_view
FormulaError::ToString() const
{
  switch (category_) {
    case Category::Ref:
      return "#REF!";
    case Category::Value:
      return "#VALUE!";
    case Category::Div0:
      return "#DIV/0!";
  }
  return {};
}

ostream&
operator<<(ostream& output, FormulaError fe)
{
  return output << fe.ToString();
}

class ICellImpl;
using ICellImplPtr = shared_ptr<ICellImpl>;

class ICellId : weak_ptr<ICellImpl>
{
public:
  ICellId() = default;
  ICellId(ICellImplPtr ptr)
    : weak_ptr<ICellImpl>(ptr)
  {}

  ICellImplPtr Get() const { return lock(); }
  bool operator<(const ICellId& other) const { return owner_before(other); }
  bool operator==(const ICellId& other) const { return !owner_before(other) && !other.owner_before(*this); }
};

class ICellImpl : public ICell
{
public:
  static ICellImplPtr Create(ISheet& owner);

  void AssertCircularDependency(const vector<Position>& positions) const;

  ICellId GetId() const { return id_; }

  Value GetValue() const override;
  string GetText() const override;
  vector<Position> GetReferencedCells() const override;

  void SetText(string text);

  IFormula* GetFormula() const;
  void SetFormula(const IFormula& formula);

  void Invalidate(bool referenced);

  void AddDependencyFrom(const ICellId& cell_id);
  void RemoveDependencyFrom(const ICellId& cell_id);

  void AddDependencyTo(const ICellId& cell_id);
  void RemoveDependencyTo(const ICellId& cell_id);

private:
  ICellImpl(ISheet& owner);
  void SetId(ICellId id) { id_ = id; }

  static ICellImpl* GetImpl(ICell* cell) { return static_cast<ICellImpl*>(cell); }

  ISheet& owner_;

  string text_;
  mutable optional<Value> cached_value_;

  mutable ICellId id_;

  set<ICellId> deps_from_;
  set<ICellId> deps_to_;

  unique_ptr<IFormula> formula_;
};

class ISheetImpl : public ISheet
{
public:
  ISheetImpl();

  virtual void SetCell(Position pos, std::string text) override;
  virtual const ICell* GetCell(Position pos) const override;
  virtual ICell* GetCell(Position pos) override;

  virtual void ClearCell(Position pos) override;

  virtual void InsertRows(int before, int count = 1) override;
  virtual void InsertCols(int before, int count = 1) override;

  virtual void DeleteRows(int first, int count = 1) override;
  virtual void DeleteCols(int first, int count = 1) override;

  virtual Size GetPrintableSize() const override;
  virtual void PrintValues(std::ostream& output) const override;
  virtual void PrintTexts(std::ostream& output) const override;

private:
  void AssertValidPosition(const Position& pos) const;

  Table<ICellImplPtr> table_;
};

ICellImplPtr
ICellImpl::Create(ISheet& owner)
{
  ICellImplPtr res = shared_ptr<ICellImpl>(new ICellImpl(owner));
  res->SetId(res);
  return res;
}

void
ICellImpl::AssertCircularDependency(const vector<Position>& positions) const
{
  set<ICellId> cache;

  vector<ICellId> queue;
  queue.reserve(positions.size());
  for (const auto& position : positions) {
    if (auto cell = owner_.GetCell(position)) {
      queue.push_back(GetImpl(cell)->GetId());
    }
  }

  for (size_t i = 0; i < queue.size(); ++i) {
    if (queue[i] == id_) {
      throw CircularDependencyException("Circular dependency detected");
    }
    if (cache.insert(queue[i]).second) {
      if (auto cell = queue[i].Get()) {
        queue.insert(end(queue), begin(cell->deps_from_), end(cell->deps_from_));
      }
    }
  }
}

ICellImpl::ICellImpl(ISheet& owner)
  : owner_(owner)
{}

void
ICellImpl::AddDependencyFrom(const ICellId& cell_id)
{
  deps_from_.insert(cell_id);
}

void
ICellImpl::RemoveDependencyFrom(const ICellId& cell_id)
{
  auto it = deps_from_.find(cell_id);
  if (it != end(deps_from_)) {
    deps_from_.erase(it);
  }
}

void
ICellImpl::AddDependencyTo(const ICellId& cell_id)
{
  deps_to_.insert(cell_id);
}

void
ICellImpl::RemoveDependencyTo(const ICellId& cell_id)
{
  auto it = deps_to_.find(cell_id);
  if (it != end(deps_to_)) {
    deps_to_.erase(it);
  }
}

ICell::Value
ICellImpl::GetValue() const
{
  auto set_cached_val = [this](const auto& val) { cached_value_ = val; };
  if (!cached_value_) {
    if (auto formula = GetFormula()) {
      auto formula_val = formula->Evaluate(owner_);
      visit(set_cached_val, formula_val);
    } else {
      visit(set_cached_val, ParseValue(text_));
    }
  }
  return *cached_value_;
}

vector<Position>
ICellImpl::GetReferencedCells() const
{
  return formula_ ? formula_->GetReferencedCells() : vector<Position>();
}

void
ICellImpl::SetText(string text)
{
  if (text == text_) {
    return;
  }

  if (!text.empty() && text.front() == kFormulaSign) {
    auto formula = ParseFormula(string(next(begin(text)), end(text)));
    auto referenced_positions = formula->GetReferencedCells();

    AssertCircularDependency(referenced_positions);

    Invalidate(false);
    deps_from_.clear();

    formula_ = move(formula);
    text_ = move(text);
    for (auto referenced_pos : referenced_positions) {
      auto referenced_cell = owner_.GetCell(referenced_pos);
      if (!referenced_cell) {
        owner_.SetCell(referenced_pos, "");
        referenced_cell = owner_.GetCell(referenced_pos);
      }
      AddDependencyFrom(GetImpl(referenced_cell)->GetId());
      GetImpl(referenced_cell)->AddDependencyTo(GetId());
    }
  } else {
    Invalidate(false);
    formula_.reset();
    deps_from_.clear();
    text_ = move(text);
  }
}

IFormula*
ICellImpl::GetFormula() const
{
  return formula_.get();
}

void
ICellImpl::SetFormula(const IFormula& formula)
{
  text_ = "="s + formula.GetExpression();
}

void
ICellImpl::Invalidate(bool referenced)
{
  if (referenced) {
    for (auto dep_from_it = begin(deps_from_); dep_from_it != end(deps_from_);) {
      if (dep_from_it->Get()) {
        ++dep_from_it;
      } else {
        dep_from_it = deps_from_.erase(dep_from_it);
      }
    }
  }

  for (auto& dep_to : deps_to_) {
    if (auto pointing_cell = dep_to.Get()) {
      pointing_cell->Invalidate(false);
    }
  }

  cached_value_.reset();
}

string
ICellImpl::GetText() const
{
  return text_;
}

ISheetImpl::ISheetImpl() = default;

void
ISheetImpl::SetCell(Position pos, string text)
{
  AssertValidPosition(pos);
  if (!table_.IsInside(pos)) {
    table_.Grow(Size{ pos.row + 1, pos.col + 1 });
  }

  auto& existing_cell = table_(pos);
  if (!existing_cell) {
    existing_cell = ICellImpl::Create(*this);
  }
  existing_cell->SetText(move(text));
}

const ICell*
ISheetImpl::GetCell(Position pos) const
{
  AssertValidPosition(pos);
  if (table_.IsInside(pos)) {
    if (auto res = table_.GetAt(pos)) {
      return res->get();
    }
  }
  return nullptr;
}

ICell*
ISheetImpl::GetCell(Position pos)
{
  AssertValidPosition(pos);
  if (table_.IsInside(pos)) {
    if (auto res = table_.GetAt(pos)) {
      return res->get();
    }
  }
  return nullptr;
}

void
ISheetImpl::ClearCell(Position pos)
{
  AssertValidPosition(pos);
  if (table_.IsInside(pos) && table_.GetAt(pos)) {
    table_(pos).reset();
  }
}

void
ISheetImpl::InsertRows(int before, int count)
{
  table_.InsertRows(before, count);
  table_.ForEach([before, count](auto, auto, const ICellImplPtr* ptr_cell) {
    if (!ptr_cell) {
      return;
    }

    auto cell = *ptr_cell;
    if (cell && cell->GetFormula()) {
      auto formula = cell->GetFormula();
      switch (formula->HandleInsertedRows(before, count)) {
        case IFormula::HandlingResult::ReferencesChanged:
          cell->Invalidate(true);
          [[fallthrough]];
        case IFormula::HandlingResult::ReferencesRenamedOnly:
          cell->SetFormula(*formula);
          break;
        default:
          break;
      }
    }
  });
}

void
ISheetImpl::InsertCols(int before, int count)
{
  table_.InsertCols(before, count);
  table_.ForEach([before, count](auto, auto, const ICellImplPtr* ptr_cell) {
    if (!ptr_cell) {
      return;
    }

    auto cell = *ptr_cell;
    if (cell && cell->GetFormula()) {
      auto formula = cell->GetFormula();
      switch (formula->HandleInsertedCols(before, count)) {
        case IFormula::HandlingResult::ReferencesChanged:
          cell->Invalidate(true);
          [[fallthrough]];
        case IFormula::HandlingResult::ReferencesRenamedOnly:
          cell->SetFormula(*formula);
          break;
        default:
          break;
      }
    }
  });
}

void
ISheetImpl::DeleteRows(int first, int count)
{
  table_.DeleteRows(first, count);
  table_.ForEach([first, count](auto, auto, const ICellImplPtr* ptr_cell) {
    if (!ptr_cell) {
      return;
    }

    auto cell = *ptr_cell;
    if (cell && cell->GetFormula()) {
      auto formula = cell->GetFormula();
      switch (formula->HandleDeletedRows(first, count)) {
        case IFormula::HandlingResult::ReferencesChanged:
          cell->Invalidate(true);
          [[fallthrough]];
        case IFormula::HandlingResult::ReferencesRenamedOnly:
          cell->SetFormula(*formula);
          break;
        default:
          break;
      }
    }
  });
}

void
ISheetImpl::DeleteCols(int first, int count)
{
  table_.DeleteCols(first, count);
  table_.ForEach([first, count](auto, auto, const ICellImplPtr* ptr_cell) {
    if (!ptr_cell) {
      return;
    }

    auto cell = *ptr_cell;
    if (cell && cell->GetFormula()) {
      auto formula = cell->GetFormula();
      switch (formula->HandleDeletedCols(first, count)) {
        case IFormula::HandlingResult::ReferencesChanged:
          cell->Invalidate(true);
          [[fallthrough]];
        case IFormula::HandlingResult::ReferencesRenamedOnly:
          cell->SetFormula(*formula);
          break;
        default:
          break;
      }
    }
  });
}

Size
ISheetImpl::GetPrintableSize() const
{
  int rows = 0;
  int cols = 0;
  table_.ForEach([&rows, &cols](auto i, auto j, const ICellImplPtr* cell_ptr) {
    const int multiplier = cell_ptr && *cell_ptr && !(*cell_ptr)->GetText().empty();
    rows = max(rows, (i + 1) * multiplier);
    cols = max(cols, (j + 1) * multiplier);
  });

  return Size{ rows, cols };
}

void
ISheetImpl::PrintValues(ostream& output) const
{
  const auto size = GetPrintableSize();
  for (int i = 0; i < size.rows; ++i) {
    for (int j = 0; j < size.cols; ++j) {
      if (j != 0) {
        output << '\t';
      }
      if (const ICellImplPtr* cell_ptr = table_.GetAt(i, j)) {
        if (cell_ptr->get()) {
          visit([&output](const auto& val) { output << val; }, (*cell_ptr)->GetValue());
        }
      }
    }
    output << '\n';
  }
}

void
ISheetImpl::PrintTexts(ostream& output) const
{
  const auto size = GetPrintableSize();
  for (int i = 0; i < size.rows; ++i) {
    for (int j = 0; j < size.cols; ++j) {
      if (j != 0) {
        output << '\t';
      }
      if (const ICellImplPtr* cell_ptr = table_.GetAt(i, j)) {
        if (cell_ptr->get()) {
          output << (*cell_ptr)->GetText();
        }
      }
    }
    output << '\n';
  }
}

void
ISheetImpl::AssertValidPosition(const Position& pos) const
{
  if (!pos.IsValid()) {
    throw InvalidPositionException("Invalid position");
  }
}

std::unique_ptr<ISheet>
CreateSheet()
{
  return make_unique<ISheetImpl>();
}
