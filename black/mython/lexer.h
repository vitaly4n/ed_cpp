#pragma once

#include <iosfwd>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

class TestRunner;

namespace Parse {

namespace TokenType {
struct Number
{
  int value;
};

struct Id
{
  std::string value;
};

struct Char
{
  char value;
};

struct String
{
  std::string value;
};

struct Class
{};
struct Return
{};
struct If
{};
struct Else
{};
struct Def
{};
struct Newline
{};
struct Print
{};
struct Indent
{};
struct Dedent
{};
struct Eof
{};
struct And
{};
struct Or
{};
struct Not
{};
struct Eq
{};
struct NotEq
{};
struct LessOrEq
{};
struct GreaterOrEq
{};
struct None
{};
struct True
{};
struct False
{};
}

using TokenBase = std::variant<TokenType::Number,
                               TokenType::Id,
                               TokenType::Char,
                               TokenType::String,
                               TokenType::Class,
                               TokenType::Return,
                               TokenType::If,
                               TokenType::Else,
                               TokenType::Def,
                               TokenType::Newline,
                               TokenType::Print,
                               TokenType::Indent,
                               TokenType::Dedent,
                               TokenType::And,
                               TokenType::Or,
                               TokenType::Not,
                               TokenType::Eq,
                               TokenType::NotEq,
                               TokenType::LessOrEq,
                               TokenType::GreaterOrEq,
                               TokenType::None,
                               TokenType::True,
                               TokenType::False,
                               TokenType::Eof>;

struct Token : TokenBase
{
  using TokenBase::TokenBase;

  template<typename T>
  bool Is() const
  {
    return std::holds_alternative<T>(*this);
  }

  template<typename T>
  const T& As() const
  {
    return std::get<T>(*this);
  }

  template<typename T>
  const T* TryAs() const
  {
    return std::get_if<T>(this);
  }
};

bool
operator==(const Token& lhs, const Token& rhs);
std::ostream&
operator<<(std::ostream& os, const Token& rhs);

class LexerError : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

class Lexer
{
public:
  explicit Lexer(std::istream& input);

  const Token& CurrentToken() const;
  Token NextToken();

  template<typename T>
  const T& Expect() const
  {
    const Token& token = CurrentToken();
    if (!token.Is<T>()) {
      throw LexerError("different token expected");
    }
    return token.As<T>();
  }

  template<typename T, typename U>
  void Expect(const U& value) const
  {
    const Token& token = CurrentToken();
    if (!token.Is<T>() || token.As<T>().value != value) {
      throw LexerError("different token expected");
    }
  }

  template<typename T>
  const T& ExpectNext()
  {
    NextToken();
    return Expect<T>();
  }

  template<typename T, typename U>
  void ExpectNext(const U& value)
  {
    NextToken();
    Expect<T>(value);
  }

private:
  Token ReadToken();

  std::istream& input_;
  std::vector<Token> tokens_;

  unsigned current_level_ = 0;
  unsigned previous_level_ = 0;
};

void
RunLexerTests(TestRunner& test_runner);

} /* namespace Parse */
