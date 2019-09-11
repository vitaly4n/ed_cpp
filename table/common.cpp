#include "common.h"

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
  return row <= kMaxRows && col <= kMaxCols;
}

string
Position::ToString() const
{
  // TODO:
  return "";
}

Position
Position::FromString(string_view str)
{
  // TODO:
  return { 0, 0 };
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

ISheetPtr
CreateSheet()
{
  // TODO:
  return nullptr;
}

class ICellImpl : public ICell
{
public:
  Value GetValue() const override;
  string GetText() const override;
  vector<Position> GetReferencedCells() const override;
};

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

void
ISheetImpl::SetCell(Position pos, string text)
{
  // TODO:
}

const ICell*
ISheetImpl::GetCell(Position pos) const
{
  // TODO:
  return nullptr;
}

ICell*
ISheetImpl::GetCell(Position pos)
{
  // TODO:
  return nullptr;
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
  return {0, 0};
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
