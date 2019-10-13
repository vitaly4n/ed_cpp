#include "formula.h"

#include "utils.h"

#include "FormulaBaseListener.h"
#include "FormulaLexer.h"
#include "FormulaParser.h"

#include <cmath>
#include <string>

using namespace std;

class FormulaNode;
using FormulaNodePtr = unique_ptr<FormulaNode>;
class FormulaNode
{
public:
  enum class Type
  {
    Unary,
    Add,
    Sub,
    Mul,
    Div,
    Addr,
    Num
  };

  FormulaNode(Type type)
    : type_(type)
  {}

  virtual ~FormulaNode() = default;
  virtual IFormula::Value Evaluate(const ISheet& sheet) const = 0;
  void Out(ostream& os, bool pars) const
  {
    if (pars) {
      os << '(';
    }
    Out(os);
    if (pars) {
      os << ')';
    }
  }

  virtual void Out(ostream& os) const = 0;

  Type GetType() const { return type_; }

private:
  Type type_;
};

class UnaryOpNode : public FormulaNode
{
public:
  UnaryOpNode(FormulaNodePtr operand, int sign)
    : FormulaNode(Type::Unary)
    , operand_(move(operand))
    , sign_(sign)
  {}

  IFormula::Value Evaluate(const ISheet& sheet) const override
  {
    auto res = operand_->Evaluate(sheet);
    if (holds_alternative<FormulaError>(res)) {
      return get<FormulaError>(res);
    } else {
      return sign_ * get<double>(res);
    }
  }

  void Out(ostream& os) const override
  {
    os << (sign_ < 0 ? '-' : '+');
    operand_->Out(os, IsAny(operand_->GetType(), Type::Add, Type::Sub));
  }

private:
  FormulaNodePtr operand_;
  int sign_ = 1;
};

class BinaryOpNode : public FormulaNode
{
public:
  BinaryOpNode(FormulaNodePtr lhs, FormulaNodePtr rhs, Type type)
    : FormulaNode(type)
    , lhs_(move(lhs))
    , rhs_(move(rhs))
  {}

  virtual IFormula::Value ApplyOp(double lhs, double rhs) const = 0;
  IFormula::Value Evaluate(const ISheet& sheet) const override final
  {
    const auto lhs_res = lhs_->Evaluate(sheet);
    const auto rhs_res = rhs_->Evaluate(sheet);
    if (holds_alternative<FormulaError>(lhs_res)) {
      return lhs_res;
    }
    if (holds_alternative<FormulaError>(rhs_res)) {
      return rhs_res;
    }
    const double res = get<double>(ApplyOp(get<double>(lhs_res), get<double>(rhs_res)));
    return isfinite(res) ? IFormula::Value(res) : IFormula::Value(FormulaError::Category::Div0);
  }

  const FormulaNode* Left() const { return lhs_.get(); }
  const FormulaNode* Right() const { return rhs_.get(); }

private:
  FormulaNodePtr lhs_;
  FormulaNodePtr rhs_;
};

class AddOpNode : public BinaryOpNode
{
public:
  AddOpNode(FormulaNodePtr lhs, FormulaNodePtr rhs)
    : BinaryOpNode(move(lhs), move(rhs), Type::Add)
  {}

  IFormula::Value ApplyOp(double lhs, double rhs) const override { return lhs + rhs; }
  void Out(ostream& os) const override
  {
    Left()->Out(os, false);
    os << '+';
    Right()->Out(os, false);
  }
};

class SubOpNode : public BinaryOpNode
{
public:
  SubOpNode(FormulaNodePtr lhs, FormulaNodePtr rhs)
    : BinaryOpNode(move(lhs), move(rhs), Type::Sub)
  {}

  IFormula::Value ApplyOp(double lhs, double rhs) const override { return lhs - rhs; }
  void Out(ostream& os) const override
  {
    Left()->Out(os, false);
    os << '-';
    Right()->Out(os, IsAny(Right()->GetType(), Type::Add, Type::Sub));
  }
};

class MulOpNode : public BinaryOpNode
{
public:
  MulOpNode(FormulaNodePtr lhs, FormulaNodePtr rhs)
    : BinaryOpNode(move(lhs), move(rhs), Type::Mul)
  {}

  IFormula::Value ApplyOp(double lhs, double rhs) const override { return lhs * rhs; }
  void Out(ostream& os) const override
  {
    Left()->Out(os, IsAny(Left()->GetType(), Type::Add, Type::Sub));
    os << '*';
    Right()->Out(os, IsAny(Right()->GetType(), Type::Add, Type::Sub));
  }
};

class DivOpNode : public BinaryOpNode
{
public:
  DivOpNode(FormulaNodePtr lhs, FormulaNodePtr rhs)
    : BinaryOpNode(move(lhs), move(rhs), Type::Div)
  {}

  IFormula::Value ApplyOp(double lhs, double rhs) const override { return lhs / rhs; }
  void Out(ostream& os) const override
  {
    Left()->Out(os, IsAny(Left()->GetType(), Type::Add, Type::Sub));
    os << '/';
    Right()->Out(os, IsAny(Right()->GetType(), Type::Add, Type::Sub, Type::Mul, Type::Div));
  }
};

class AddrNode : public FormulaNode
{
public:
  AddrNode(string_view strpos)
    : FormulaNode(Type::Addr)
    , position_(Position::FromString(strpos))
  {
    if (!position_.IsValid()) {
      throw FormulaException("Trying to create a reference to invalid cell");
    }
  }

  IFormula::Value Evaluate(const ISheet& sheet) const override
  {
    if (!position_.IsValid()) {
      return FormulaError::Category::Ref;
    }

    const auto cell = sheet.GetCell(position_);
    if (!cell) {
      return 0.;
    }

    return visit(
      [](const auto& value) -> IFormula::Value {
        using T = remove_cv_t<remove_reference_t<decltype(value)>>;
        if constexpr (is_same<string, T>::value) {
          return value.empty() ? IFormula::Value(0) : IFormula::Value(FormulaError::Category::Value);
        } else {
          return value;
        }
      },
      cell->GetValue());
  }

  void Out(ostream& os) const override
  {
    if (position_.IsValid()) {
      os << position_.ToString();
    } else {
      os << FormulaError(FormulaError::Category::Ref);
    }
  }

  const Position& GetPosition() const { return position_; }
  void SetPosition(Position position) { position_ = position; }

private:
  Position position_;
};

class NumberNode : public FormulaNode
{
public:
  NumberNode(double val)
    : FormulaNode(Type::Num)
    , val_(val)
  {}

  IFormula::Value Evaluate(const ISheet&) const override { return val_; }
  void Out(ostream& os) const override { os << val_; }

private:
  double val_ = 0.;
};

struct AddrNodePtrLess
{
  bool operator()(const AddrNode* lhs, const AddrNode* rhs) const { return lhs->GetPosition() < rhs->GetPosition(); }
};

struct AddrNodePtrEqual
{
  bool operator()(const AddrNode* lhs, const AddrNode* rhs) const { return lhs->GetPosition() == rhs->GetPosition(); }
};

class ANTLRFormulaListener : public FormulaBaseListener
{
public:
  FormulaNodePtr GetTree() { return FormulaNodePtr(move(ast_)); }
  vector<AddrNode*> GetAddrNodes()
  {
    sort(begin(addr_nodes_), end(addr_nodes_), AddrNodePtrLess());
    return move(addr_nodes_);
  }

  void enterMain(FormulaParser::MainContext*) override
  {
    ast_.reset();
    children_.clear();
  }

  void exitMain(FormulaParser::MainContext*) override
  {
    if (children_.size() != 1) {
      throw FormulaException("Wrong number of root tokens");
    }

    ast_ = move(children_.back());
    children_.clear();
  }

  void exitUnaryOp(FormulaParser::UnaryOpContext* ctx) override
  {
    FormulaNodePtr operand = move(children_.back());
    children_.pop_back();

    int sign = ctx->ADD() ? 1 : -1;
    children_.push_back(make_unique<UnaryOpNode>(move(operand), sign));
  }

  void exitBinaryOp(FormulaParser::BinaryOpContext* ctx) override
  {
    FormulaNodePtr rhs = move(children_.back());
    children_.pop_back();
    FormulaNodePtr lhs = move(children_.back());
    children_.pop_back();

    FormulaNodePtr binary_node;
    if (ctx->ADD()) {
      binary_node = make_unique<AddOpNode>(move(lhs), move(rhs));
    } else if (ctx->SUB()) {
      binary_node = make_unique<SubOpNode>(move(lhs), move(rhs));
    } else if (ctx->MUL()) {
      binary_node = make_unique<MulOpNode>(move(lhs), move(rhs));
    } else if (ctx->DIV()) {
      binary_node = make_unique<DivOpNode>(move(lhs), move(rhs));
    }
    children_.push_back(move(binary_node));
  }

  virtual void enterCell(FormulaParser::CellContext* ctx) override
  {
    auto addr_node = make_unique<AddrNode>(ctx->CELL()->getText());
    addr_nodes_.push_back(addr_node.get());
    children_.push_back(move(addr_node));
  }

  virtual void exitLiteral(FormulaParser::LiteralContext* ctx) override
  {
    children_.push_back(make_unique<NumberNode>(stod(ctx->NUMBER()->getText())));
  }

private:
  FormulaNodePtr ast_;
  deque<FormulaNodePtr> children_;
  vector<AddrNode*> addr_nodes_;
};

class BailErrorListener : public antlr4::BaseErrorListener
{
public:
  void syntaxError(antlr4::Recognizer* /* recognizer */,
                   antlr4::Token* /* offendingSymbol */,
                   size_t /* line */,
                   size_t /* charPositionInLine */,
                   const string& msg,
                   exception_ptr /* e */
                   ) override
  {
    throw FormulaException("Error when lexing: " + msg);
  }
};

class IFormulaImpl : public IFormula
{
public:
  IFormulaImpl(FormulaNodePtr ast, vector<AddrNode*> addr_nodes);

  Value Evaluate(const ISheet& sheet) const override;

  string GetExpression() const override;

  vector<Position> GetReferencedCells() const override;

  HandlingResult HandleInsertedRows(int before, int count = 1) override;
  HandlingResult HandleInsertedCols(int before, int count = 1) override;

  HandlingResult HandleDeletedRows(int first, int count = 1) override;
  HandlingResult HandleDeletedCols(int first, int count = 1) override;

  void UpdateReferencedCells();

private:
  FormulaNodePtr ast_;
  string expression_;
  vector<AddrNode*> addr_nodes_;
  vector<Position> referenced_cells_;
};

std::unique_ptr<IFormula>
ParseFormula(std::string expression)
{
  ostringstream stdinput;
  antlr4::ANTLRInputStream input(expression);
  FormulaLexer lexer(&input);

  BailErrorListener error_listener;
  lexer.removeErrorListeners();
  lexer.addErrorListener(&error_listener);

  antlr4::CommonTokenStream tokens(&lexer);

  FormulaParser parser(&tokens);
  auto error_handler = std::make_shared<antlr4::BailErrorStrategy>();
  parser.setErrorHandler(error_handler);
  parser.removeErrorListeners();

  ANTLRFormulaListener listener;
  FormulaNodePtr ast;
  try {
    antlr4::tree::ParseTree* tree = parser.main();
    antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);

    ast = listener.GetTree();
    if (!ast) {
      throw FormulaException("Could not build AST tree");
    }
  } catch (const FormulaException&) {
    throw;
  } catch (...) {
    throw FormulaException("Could not build AST tree");
  }
  return make_unique<IFormulaImpl>(move(ast), listener.GetAddrNodes());
}

IFormulaImpl::IFormulaImpl(FormulaNodePtr ast, vector<AddrNode*> addr_nodes)
  : ast_(move(ast))
  , addr_nodes_(move(addr_nodes))
{
  UpdateReferencedCells();
}

IFormula::Value
IFormulaImpl::Evaluate(const ISheet& sheet) const
{
  return ast_->Evaluate(sheet);
}

string
IFormulaImpl::GetExpression() const
{
  return expression_;
}

vector<Position>
IFormulaImpl::GetReferencedCells() const
{
  return referenced_cells_;
}

IFormula::HandlingResult
IFormulaImpl::HandleInsertedRows(int before, int count)
{
  auto res = IFormula::HandlingResult::NothingChanged;
  for (auto* addr_node : addr_nodes_) {
    auto pos = addr_node->GetPosition();
    if (pos.row >= before) {
      pos.row += count;
      res = IFormula::HandlingResult::ReferencesRenamedOnly;
      addr_node->SetPosition(pos);
    }
  }
  if (res != IFormula::HandlingResult::NothingChanged) {
    UpdateReferencedCells();
  }
  return res;
}

IFormula::HandlingResult
IFormulaImpl::HandleInsertedCols(int before, int count)
{
  auto res = IFormula::HandlingResult::NothingChanged;
  for (auto* addr_node : addr_nodes_) {
    auto pos = addr_node->GetPosition();
    if (pos.col >= before) {
      pos.col += count;
      res = IFormula::HandlingResult::ReferencesRenamedOnly;
      addr_node->SetPosition(pos);
    }
  }
  if (res != IFormula::HandlingResult::NothingChanged) {
    UpdateReferencedCells();
  }
  return res;
}

IFormula::HandlingResult
IFormulaImpl::HandleDeletedRows(int first, int count)
{
  auto res = IFormula::HandlingResult::NothingChanged;
  for (auto* addr_node : addr_nodes_) {
    auto pos = addr_node->GetPosition();
    if (pos.row >= first) {
      if (pos.row < first + count) {
        res = IFormula::HandlingResult::ReferencesChanged;
        pos.row = -1;
        pos.col = -1;
        addr_node->SetPosition(pos);
      } else {
        if (res == IFormula::HandlingResult::NothingChanged) {
          res = IFormula::HandlingResult::ReferencesRenamedOnly;
        }
        pos.row -= count;
        addr_node->SetPosition(pos);
      }
    }
  }
  if (res != IFormula::HandlingResult::NothingChanged) {
    UpdateReferencedCells();
  }
  return res;
}

IFormula::HandlingResult
IFormulaImpl::HandleDeletedCols(int first, int count)
{
  auto res = IFormula::HandlingResult::NothingChanged;
  for (auto* addr_node : addr_nodes_) {
    auto pos = addr_node->GetPosition();
    if (pos.col >= first) {
      if (pos.col < first + count) {
        res = IFormula::HandlingResult::ReferencesChanged;
        pos.row = -1;
        pos.col = -1;
        addr_node->SetPosition(pos);
      } else {
        if (res == IFormula::HandlingResult::NothingChanged) {
          res = IFormula::HandlingResult::ReferencesRenamedOnly;
        }
        pos.col -= count;
        addr_node->SetPosition(pos);
      }
    }
  }
  if (res != IFormula::HandlingResult::NothingChanged) {
    UpdateReferencedCells();
  }
  return res;
}

void
IFormulaImpl::UpdateReferencedCells()
{
  referenced_cells_.clear();
  referenced_cells_.reserve(addr_nodes_.size());
  for (const auto& addr_node : addr_nodes_) {
    if (addr_node && addr_node->GetPosition().IsValid()) {
      referenced_cells_.push_back(addr_node->GetPosition());
    }
  }
  referenced_cells_.erase(unique(begin(referenced_cells_), end(referenced_cells_)), end(referenced_cells_));

  ostringstream os;
  ast_->Out(os);
  expression_ = os.str();
}
