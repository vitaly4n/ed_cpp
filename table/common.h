#pragma once

#include <iosfwd>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// Position of a cell. Indexing from 0.
struct Position
{
  int row = 0;
  int col = 0;

  bool operator==(const Position& rhs) const;
  bool operator<(const Position& rhs) const;

  bool IsValid() const;
  std::string ToString() const;

  static Position FromString(std::string_view str);

  static const int kMaxRows = 16384;
  static const int kMaxCols = 16384;
};

struct Size
{
  int rows = 0;
  int cols = 0;

  bool operator==(const Size& rhs) const;
};

// Describes an error which can occur during formula parsing or evaluation
class FormulaError
{
public:
  enum class Category
  {
    Ref,   // reference to non-existing cell
    Value, // a cell cannot be converted to a number
    Div0,  // devision by 0
  };

  FormulaError(Category category);

  Category GetCategory() const;

  bool operator==(FormulaError rhs) const;

  std::string_view ToString() const;

private:
  Category category_;
};

std::ostream&
operator<<(std::ostream& output, FormulaError fe);

// Invalid cell position
class InvalidPositionException : public std::out_of_range
{
public:
  using std::out_of_range::out_of_range;
};

// Invaliid expression syntax
class FormulaException : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

// Cyclic dependency in a formula
class CircularDependencyException : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

// During insertions into a table some cell positions become invalid
class TableTooBigException : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

class ICell
{
public:
  // either text of a cell, or evaluated formula value, or a error message
  using Value = std::variant<std::string, double, FormulaError>;

  virtual ~ICell() = default;

  // Returns an output value of a cell.
  // In case of a text cell it is a text itself (without escaping sequences). In
  // case of a formula cell - the evaluated value
  virtual Value GetValue() const = 0;

  // Returns an internal text of a cell
  // For a text cell it is a text itself (possibly with escaping sequences).
  // For a formula cell it is a formula expression
  virtual std::string GetText() const = 0;

  // Returns a list of cells which are requeired for formula evaluation.
  // The list is sorted in ascending order and does not contain any duplicates.
  // In case of a text cell the list would be empty.
  virtual std::vector<Position> GetReferencedCells() const = 0;
};

inline constexpr char kFormulaSign = '=';
inline constexpr char kEscapeSign = '\'';

class ISheet
{
public:
  virtual ~ISheet() = default;

  // Set cell's content. If a text is started with "=" it is interpreted
  // as a formula. If a formula syntax is not valid an exception FormulaException
  // is thrown and cell's content is not changed. If a formula leads to a
  // circular dependency CircularDependencyException exception is thrown
  // and cell's content is not changed.
  // Note:
  // * If a content contains only "=" symbol it is not interpreted as a formula
  // * If a content starts with "'" it will not be shown in GetValue (it can
  //   be used to assign a text to a cell which starts with "=" but should not
  //   be interpreted as a formula
  virtual void SetCell(Position pos, std::string text) = 0;

  // Returns a pointer to cell or nullptr if a cell does not exist
  virtual const ICell* GetCell(Position pos) const = 0;
  virtual ICell* GetCell(Position pos) = 0;

  // Clears the cell. Following call of GetCell() will return either a nullptr
  // or a cell without content
  virtual void ClearCell(Position pos) = 0;

  // Inserts a given number of rows/columns before a row/column with a given index.
  // All references in expressions remain actual and point to the same cells (not by indices!).
  // TableTooBigException exception is thrown in case if table size libit
  // would be exceeded during insertion, table is not changed in this case.
  virtual void InsertRows(int before, int count = 1) = 0;
  virtual void InsertCols(int before, int count = 1) = 0;

  // Delete a given number of empty rows/columns starting with a row/column
  // with a given index. All references in expressions remain actual and point
  // to the same cells.
  virtual void DeleteRows(int first, int count = 1) = 0;
  virtual void DeleteCols(int first, int count = 1) = 0;

  // Computes a printable area of the table.
  virtual Size GetPrintableSize() const = 0;

  // Prints the whole table to a given output stream. Columns are separated
  // by tabulation sign. After every row a EOL is printed.
  virtual void PrintValues(std::ostream& output) const = 0;
  virtual void PrintTexts(std::ostream& output) const = 0;
};

using ISheetPtr = std::unique_ptr<ISheet>;

// Create an empty table
ISheetPtr
CreateSheet();
