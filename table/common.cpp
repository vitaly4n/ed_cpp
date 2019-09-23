#include "common.h"
#include "formula.h"
#include "table.h"

#include <algorithm>
#include <charconv>
#include <deque>
#include <set>
#include <sstream>
#include <utility>

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
  return row >= 0 && col >= 0 && row <= kMaxRows && col <= kMaxCols;
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
class ICellImpl : public ICell
{
public:
  static ICellImplPtr Create(ISheet& owner, const Position& pos, string text);
  ICellImpl(ISheet& owner, const Position& pos, string text);

  Value GetValue() const override;
  string GetText() const override;
  vector<Position> GetReferencedCells() const override;

  void SetText(string text);

  IFormula* GetFormula() { return formula_.get(); }
  const IFormula* GetFormula() const { return formula_.get(); }

  void UpdateReferencedCells(ISheet& sheet, const Position& thisPos, vector<Position> new_positions);

  void AddReferencingCell(const Position& pos);
  void RemoveReferencingCell(const Position& pos);

  void Invalidate();

private:
  static ICellImpl* GetImpl(ICell* cell) { return static_cast<ICellImpl*>(cell); }

  ISheet& owner_;

  string text_;
  IFormulaPtr formula_;
  mutable optional<Value> cached_value_;

  set<Position> referenced_cells_;
  set<Position> referencing_cells_;
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
ICellImpl::Create(ISheet& owner, const Position& pos, string text)
{
  return make_shared<ICellImpl>(owner, pos, move(text));
}

ICellImpl::ICellImpl(ISheet& owner, const Position& pos, string text)
  : owner_(owner)
  , text_(move(text))
{
  if (!text_.empty() && text_.front() == kFormulaSign) {
    formula_ = ParseFormula(string(next(begin(text_)), end(text_)));
    UpdateReferencedCells(owner, pos, formula_->GetReferencedCells());
  }
}

ICell::Value
ICellImpl::GetValue() const
{
  auto set_cached_val = [this](const auto& val) { cached_value_ = val; };

  if (!cached_value_) {
    if (formula_) {
      auto formula_val = formula_->Evaluate(owner_);
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
  return vector<Position>(begin(referenced_cells_), end(referenced_cells_));
}

void
ICellImpl::SetText(string text)
{
  text_ = move(text);
}

void
ICellImpl::UpdateReferencedCells(ISheet& sheet, const Position& this_pos, vector<Position> new_positions)
{
  set<Position> cache;
  deque<Position> queue(begin(new_positions), end(new_positions));
  while (!queue.empty()) {
    auto referenced_pos = queue.front();
    if (referenced_pos == this_pos) {
      throw CircularDependencyException("Circular dependency detected");
    }

    queue.pop_front();
    auto insert_res = cache.insert(referenced_pos);
    if (insert_res.second) {
      if (auto referenced_cell = sheet.GetCell(referenced_pos)) {
        auto subcells = referenced_cell->GetReferencedCells();
        queue.insert(end(queue), begin(subcells), end(subcells));
      }
    }
  }

  for (const auto& referenced_pos : referenced_cells_) {
    if (auto referenced_cell = sheet.GetCell(referenced_pos)) {
      GetImpl(referenced_cell)->RemoveReferencingCell(this_pos);
    }
  }

  referenced_cells_ = set<Position>(make_move_iterator(begin(new_positions)), make_move_iterator(end(new_positions)));
  for (const auto& referenced_pos : referenced_cells_) {
    if (!owner_.GetCell(referenced_pos)) {
      sheet.SetCell(referenced_pos, ""s);
    }
    auto referenced_cell = sheet.GetCell(referenced_pos);
    GetImpl(referenced_cell)->AddReferencingCell(this_pos);
  }
}

void
ICellImpl::AddReferencingCell(const Position& pos)
{
  referencing_cells_.insert(pos);
}

void
ICellImpl::RemoveReferencingCell(const Position& pos)
{
  auto it = referencing_cells_.find(pos);
  if (it != end(referencing_cells_)) {
    referencing_cells_.erase(it);
  }
}

void
ICellImpl::Invalidate()
{
  cached_value_.reset();
}

string
ICellImpl::GetText() const
{
  return formula_ ? "="s + formula_->GetExpression() : text_;
}

ISheetImpl::ISheetImpl() = default;

void
ISheetImpl::SetCell(Position pos, string text)
{
  AssertValidPosition(pos);
  if (!table_.IsInside(pos)) {
    table_.Grow(Size{ pos.row + 1, pos.col + 1 });
  }

  auto existing_cell = table_(pos);
  if (!existing_cell || existing_cell->GetText() != text) {
    table_(pos) = ICellImpl::Create(*this, pos, move(text));
  }
}

const ICell*
ISheetImpl::GetCell(Position pos) const
{
  AssertValidPosition(pos);
  return table_.IsInside(pos) ? table_(pos).get() : nullptr;
}

ICell*
ISheetImpl::GetCell(Position pos)
{
  AssertValidPosition(pos);
  return table_.IsInside(pos) ? table_(pos).get() : nullptr;
}

void
ISheetImpl::ClearCell(Position pos)
{
  AssertValidPosition(pos);
  if (table_.IsInside(pos)) {
    table_(pos) = nullptr;
  }
}

void
ISheetImpl::InsertRows(int before, int count)
{
  table_.InsertRows(before, count);
  table_.ForEach([before, count, this](auto i, auto j, auto& cell) {
    if (cell && cell->GetFormula()) {
      auto formula = cell->GetFormula();
      switch (formula->HandleInsertedRows(before, count)) {
        case IFormula::HandlingResult::ReferencesChanged:
          cell->Invalidate();
          [[fallthrough]];
        case IFormula::HandlingResult::ReferencesRenamedOnly:
          cell->UpdateReferencedCells(*this, Position{ i, j }, formula->GetReferencedCells());
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
  table_.ForEach([before, count, this](auto i, auto j, auto& cell) {
    if (cell && cell->GetFormula()) {
      auto formula = cell->GetFormula();
      switch (formula->HandleInsertedCols(before, count)) {
        case IFormula::HandlingResult::ReferencesChanged:
          cell->Invalidate();
          [[fallthrough]];
        case IFormula::HandlingResult::ReferencesRenamedOnly:
          cell->UpdateReferencedCells(*this, Position{ i, j }, formula->GetReferencedCells());
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
  table_.ForEach([first, count, this](auto i, auto j, auto& cell) {
    if (cell && cell->GetFormula()) {
      auto formula = cell->GetFormula();
      switch (formula->HandleDeletedRows(first, count)) {
        case IFormula::HandlingResult::ReferencesChanged:
          cell->Invalidate();
          [[fallthrough]];
        case IFormula::HandlingResult::ReferencesRenamedOnly:
          cell->UpdateReferencedCells(*this, Position{ i, j }, formula->GetReferencedCells());
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
  table_.ForEach([first, count, this](auto i, auto j, auto& cell) {
    if (cell && cell->GetFormula()) {
      auto formula = cell->GetFormula();
      switch (formula->HandleDeletedCols(first, count)) {
        case IFormula::HandlingResult::ReferencesChanged:
          cell->Invalidate();
          [[fallthrough]];
        case IFormula::HandlingResult::ReferencesRenamedOnly:
          cell->UpdateReferencedCells(*this, Position{ i, j }, formula->GetReferencedCells());
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
  return table_.GetSize();
}

void
ISheetImpl::PrintValues(ostream& output) const
{
  const auto size = table_.GetSize();
  for (int i = 0; i < size.rows; ++i) {
    for (int j = 0; j < size.cols; ++j) {
      if (j != 0) {
        output << '\t';
      }
      if (const auto& cell = table_(i, j)) {
        visit([&output](const auto& val) { output << val; }, cell->GetValue());
      }
    }
    output << '\n';
  }
}

void
ISheetImpl::PrintTexts(ostream& output) const
{
  const auto size = table_.GetSize();
  for (int i = 0; i < size.rows; ++i) {
    for (int j = 0; j < size.cols; ++j) {
      if (j != 0) {
        output << '\t';
      }
      if (const auto& cell = table_(i, j)) {
        output << cell->GetText();
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

ISheetPtr
CreateSheet()
{
  return make_unique<ISheetImpl>();
}
