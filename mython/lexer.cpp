#include "lexer.h"

#include <algorithm>
#include <charconv>
#include <unordered_map>

using namespace std;

namespace Parse {

optional<Token>
GetKeywordToken(string_view alnumseq)
{
  static unordered_map<string_view, Token> keyword_tokens = {
    { "class", TokenType::Class() }, { "return", TokenType::Return() }, { "if", TokenType::If() },
    { "else", TokenType::Else() },   { "def", TokenType::Def() },       { "print", TokenType::Print() },
    { "or", TokenType::Or() },       { "none", TokenType::None() },     { "and", TokenType::And() },
    { "not", TokenType::Not() },     { "True", TokenType::True() },     { "False", TokenType::False() }
  };

  auto it = keyword_tokens.find(alnumseq);
  if (it == end(keyword_tokens)) {
    return nullopt;
  }
  return it->second;
}

bool
operator==(const Token& lhs, const Token& rhs)
{
  using namespace TokenType;

  if (lhs.index() != rhs.index()) {
    return false;
  }
  if (lhs.Is<Char>()) {
    return lhs.As<Char>().value == rhs.As<Char>().value;
  } else if (lhs.Is<Number>()) {
    return lhs.As<Number>().value == rhs.As<Number>().value;
  } else if (lhs.Is<String>()) {
    return lhs.As<String>().value == rhs.As<String>().value;
  } else if (lhs.Is<Id>()) {
    return lhs.As<Id>().value == rhs.As<Id>().value;
  } else {
    return true;
  }
}

std::ostream&
operator<<(std::ostream& os, const Token& rhs)
{
  using namespace TokenType;

#define VALUED_OUTPUT(type)                                                                                            \
  do {                                                                                                                 \
    if (auto p = rhs.TryAs<type>())                                                                                    \
      return os << #type << '{' << p->value << '}';                                                                    \
  } while (false)

  VALUED_OUTPUT(Number);
  VALUED_OUTPUT(Id);
  VALUED_OUTPUT(String);
  VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type)                                                                                          \
  do {                                                                                                                 \
    if (rhs.Is<type>())                                                                                                \
      return os << #type;                                                                                              \
  } while (false)

  UNVALUED_OUTPUT(Class);
  UNVALUED_OUTPUT(Return);
  UNVALUED_OUTPUT(If);
  UNVALUED_OUTPUT(Else);
  UNVALUED_OUTPUT(Def);
  UNVALUED_OUTPUT(Newline);
  UNVALUED_OUTPUT(Print);
  UNVALUED_OUTPUT(Indent);
  UNVALUED_OUTPUT(Dedent);
  UNVALUED_OUTPUT(And);
  UNVALUED_OUTPUT(Or);
  UNVALUED_OUTPUT(Not);
  UNVALUED_OUTPUT(Eq);
  UNVALUED_OUTPUT(NotEq);
  UNVALUED_OUTPUT(LessOrEq);
  UNVALUED_OUTPUT(GreaterOrEq);
  UNVALUED_OUTPUT(None);
  UNVALUED_OUTPUT(True);
  UNVALUED_OUTPUT(False);
  UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

  return os << "Unknown token :(";
}

Lexer::Lexer(istream& input)
  : input_(input)
{}

const Token&
Lexer::CurrentToken() const
{
  if (tokens_.empty()) {
    throw LexerError("No token is read");
  }
  return *rbegin(tokens_);
}

Token
Lexer::NextToken()
{
  if (!input_) {
    return TokenType::Eof();
  }

  char c;
  input_ >> c;

  // symbols:
  switch (c) {
    case '=': {
      if (input_.peek() == '=') {
        input_.get();
        return TokenType::Eq();
      }
    }
      [[fallthrough]];
    case '>': {
      auto next_char = input_.peek();
      if (next_char == '=') {
        input_.get();
        return TokenType::GreaterOrEq();
      }
    }
      [[fallthrough]];
    case '<': {
      auto next_char = input_.peek();
      if (next_char == '=') {
        input_.get();
        return TokenType::LessOrEq();
      }
    }
      [[fallthrough]];
    case '!': {
      auto next_char = input_.peek();
      if (next_char == '=') {
        input_.get();
        return TokenType::NotEq();
      }
    }
      [[fallthrough]];
    case '.':
      [[fallthrough]];
    case ',':
      [[fallthrough]];
    case '(':
      [[fallthrough]];
    case ')':
      [[fallthrough]];
    case '+':
      [[fallthrough]];
    case '-':
      [[fallthrough]];
    case '*':
      [[fallthrough]];
    case '/':
      [[fallthrough]];
    case ':':
      return TokenType::Char{ c };
    case '\n':
      return TokenType::Newline();
    case '\0':
      return TokenType::Eof();
  }

  // keywords or ids
  if (isalpha(c)) { // todo: _ symbol at the beginnig of id
    string string_token;
    while (isalnum(input_.peek())) {
      string_token.push_back(char(input_.get()));
    }

    auto keyword_token = GetKeywordToken(string_token);
    if (keyword_token) {
      return *keyword_token;
    } else {
      return TokenType::Id{ move(string_token) };
    }
  }

  return TokenType::Eof();
}

} /* namespace Parse */
