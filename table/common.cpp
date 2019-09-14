#include "common.h"

#include <algorithm>
#include <charconv>
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
      return "Reference to non-existing cell";
    case Category::Value:
      return "A cell cannot be converted to a number";
    case Category::Div0:
      return "Devided by 0";
  }
  return {};
}

ostream&
operator<<(ostream& output, FormulaError fe)
{
  return output << fe.ToString();
}

class ICellImpl : public ICell
{
public:
  Value GetValue() const override;
  string GetText() const override;
  vector<Position> GetReferencedCells() const override;
};

class ISheetImpl : public ISheet
{
public:
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
};

ISheetPtr
CreateSheet()
{
  return make_unique<ISheetImpl>();
}

ICell::Value
ICellImpl::GetValue() const
{
  // TODO:
  return {};
}

vector<Position>
ICellImpl::GetReferencedCells() const
{
  // TODO:
  return {};
}

string
ICellImpl::GetText() const
{
  // TODO:
  return {};
}

void
ISheetImpl::SetCell(Position pos, string text)
{
  // TODO:
}

const ICell*
ISheetImpl::GetCell(Position pos) const
{
  // TODO:
  static ICellImpl cell;
  return &cell;
}

ICell*
ISheetImpl::GetCell(Position pos)
{
  // TODO:
  static ICellImpl cell;
  return &cell;
}

void
ISheetImpl::ClearCell(Position pos)
{
  // TODO:
}

void
ISheetImpl::InsertRows(int before, int count)
{
  // TODO:
}

void
ISheetImpl::InsertCols(int before, int count)
{
  // TODO:
}

void
ISheetImpl::DeleteRows(int first, int count)
{
  // TODO:
}

void
ISheetImpl::DeleteCols(int first, int count)
{
  // TODO:
}

Size
ISheetImpl::GetPrintableSize() const
{
  // TODO:
  return { 0, 0 };
}

void
ISheetImpl::PrintValues(ostream& output) const
{
  // TODO:
}

void
ISheetImpl::PrintTexts(ostream& output) const
{
  // TODO:
}
