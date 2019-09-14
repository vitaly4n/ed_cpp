#include "formula.h"

using namespace std;


class IFormulaImpl : public IFormula
{
public:
  Value Evaluate(const ISheet& sheet) const override;

  string GetExpression() const override;

  vector<Position> GetReferencedCells() const override;

  HandlingResult HandleInsertedRows(int before, int count = 1) override;
  HandlingResult HandleInsertedCols(int before, int count = 1) override;

  HandlingResult HandleDeletedRows(int first, int count = 1) override;
  HandlingResult HandleDeletedCols(int first, int count = 1) override;
};

IFormulaPtr
ParseFormula(std::string expression)
{
  return make_unique<IFormulaImpl>();
}

IFormula::Value
IFormulaImpl::Evaluate(const ISheet& sheet) const
{
  // TODO:
  return {};
}

string
IFormulaImpl::GetExpression() const
{
  // TODO:
  return {};
}

vector<Position>
IFormulaImpl::GetReferencedCells() const
{
  // TODO:
  return {};
}

IFormula::HandlingResult
IFormulaImpl::HandleInsertedRows(int before, int count)
{
  // TODO:
  return {};
}

IFormula::HandlingResult
IFormulaImpl::HandleInsertedCols(int before, int count)
{
  // TODO:
  return {};
}

IFormula::HandlingResult
IFormulaImpl::HandleDeletedRows(int first, int count)
{
  // TODO:
  return {};
}

IFormula::HandlingResult
IFormulaImpl::HandleDeletedCols(int first, int count)
{
  // TODO:
  return {};
}
