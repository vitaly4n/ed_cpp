#pragma once

#include "common.h"

#include <memory>
#include <vector>

// Formula which allows to evaluate and update arithmetic expressions.
// * Binary operations and numbers, parantheses: 1+2*3, 2.5*(2+3.5/7)
// * References to cells: A1+B2*C3
// A cell referenced by a formula can be either a formula itself or a text.
// If it is a text which can be interpreted as a number it will be interpreted
// as that number. In case of an empty text or empty cell a reference will be
// evaluated as zero.
class IFormula
{
public:
  using Value = std::variant<double, FormulaError>;

  // Insertion/deletion result
  enum class HandlingResult
  {
    NothingChanged,        // nothing has changed
    ReferencesRenamedOnly, // cell labels are updated
    ReferencesChanged      // some cell referenced by a formula has been deleted
  };

  virtual ~IFormula() = default;

  // Evaluates the formula and returns either a result of evaluation or a error.
  // In case of multiple errors either may be returned.
  virtual Value Evaluate(const ISheet& sheet) const = 0;

  // Returns an expression of a formula. Does not contain spaces or excessive
  // parantheses.
  virtual std::string GetExpression() const = 0;


  // Returns a list of referenced by the formula cells. The list is sorted in
  // ascending order and does not contain duplicates
  virtual std::vector<Position> GetReferencedCells() const = 0;

  // Updates a formula for insertion of a given number of rows/columns before
  // a row/column with a given index.
  virtual HandlingResult HandleInsertedRows(int before, int count = 1) = 0;
  virtual HandlingResult HandleInsertedCols(int before, int count = 1) = 0;

  // Updates a formula for deletion of a given number of rows/columns before
  // starting with row/column with a given index. If a referenced cell has been
  // deleted its reference in an expression is substituted by a string value of
  // an error, evaluation of expression with such substituted reference will
  // result in the same error.
  virtual HandlingResult HandleDeletedRows(int first, int count = 1) = 0;
  virtual HandlingResult HandleDeletedCols(int first, int count = 1) = 0;
};

using IFormulaPtr = std::unique_ptr<IFormula>;

// Parses an expression and returns a formula object.
// FormulaException is thrown in case of invalid syntax.
IFormulaPtr
ParseFormula(std::string expression);
