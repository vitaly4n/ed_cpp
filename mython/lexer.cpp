#include "lexer.h"

#include <algorithm>
#include <charconv>
#include <unordered_map>

using namespace std;

namespace Parse {

template<char V>
struct CharPolicy
{
  static constexpr auto value = V;
};
template<typename Policy>
bool
checkSymbolPolicy(char c)
{
  return c == Policy::value;
}

struct Digit
{};
template<>
bool
checkSymbolPolicy<Digit>(char c)
{
  return isdigit(c);
}

struct Alpha
{};
template<>
bool
checkSymbolPolicy<Alpha>(char c)
{
  return isalpha(c);
}

template<typename Policy, typename... Policies>
struct CheckPoliciesImpl
{
  static bool check(char c) { return CheckPoliciesImpl<Policy>::check(c) || CheckPoliciesImpl<Policies...>::check(c); }
};

template<typename Policy>
struct CheckPoliciesImpl<Policy>
{
  static bool check(char c) { return checkSymbolPolicy<Policy>(c); }
};

template<typename Policy, typename... Policies>
bool
checkPolicies(char c)
{
  return CheckPoliciesImpl<Policy, Policies...>::check(c);
}

optional<Token>
GetKeywordToken(string_view alnumseq)
{
  static unordered_map<string_view, Token> keyword_tokens = {
    { "class", TokenType::Class() }, { "return", TokenType::Return() }, { "if", TokenType::If() },
    { "else", TokenType::Else() },   { "def", TokenType::Def() },       { "print", TokenType::Print() },
    { "or", TokenType::Or() },       { "None", TokenType::None() },     { "and", TokenType::And() },
    { "not", TokenType::Not() },     { "True", TokenType::True() },     { "False", TokenType::False() }
  };

  auto it = keyword_tokens.find(alnumseq);
  if (it == end(keyword_tokens)) {
    return nullopt;
  }
  return it->second;
}

TokenType::Number
ReadInteger(istream& input)
{
  int number = 0;
  while (isdigit(input.peek())) {
    number *= 10;
    number += input.get() - '0';
  }
  return TokenType::Number{ number };
}

TokenType::String
ReadString(istream& input)
{
  char terminator;
  input >> terminator;

  string str;

  bool escaping = false;
  for (char nextchar = char(input.get()); escaping || nextchar != terminator; nextchar = char(input.get())) {
    escaping = !escaping && nextchar == '\\';
    str.push_back(nextchar);
  }
  return TokenType::String{ str };
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
{
  NextToken();
}

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
  auto token = ReadToken();
  tokens_.push_back(token);
  return token;
}

Token
Lexer::ReadToken()
{
  if (input_ && input_.peek() == -1) {
    input_.get();
  }

  if (!input_) {
    if (!tokens_.empty() && !CurrentToken().Is<TokenType::Eof>() && !CurrentToken().Is<TokenType::Newline>() &&
        !CurrentToken().Is<TokenType::Dedent>()) {
      return TokenType::Newline();
    }
    return TokenType::Eof();
  }

  auto IsWhiteEol = [this](char c) {
    return c == '\n' && (tokens_.empty() || CurrentToken().Is<TokenType::Newline>());
  };

  // skip whitespaces
  unsigned num_spaces = 0;

  char whitechar = char(input_.peek());
  while (whitechar == ' ' || IsWhiteEol(whitechar)) {
    input_.get();
    if (whitechar == ' ') {
      ++num_spaces;
    } else { // white newline
      num_spaces = 0;
    }
    whitechar = char(input_.peek());
  }

  // check indentation change
  if (!tokens_.empty() && CurrentToken().Is<TokenType::Newline>()) {
    current_level_ = num_spaces / 2;
  }

  if (current_level_ > previous_level_) {
    ++previous_level_;
    return TokenType::Indent();
  }
  if (current_level_ < previous_level_) {
    --previous_level_;
    return TokenType::Dedent();
  }

  char c = char(input_.get());

  // numbers
  if (checkPolicies<Digit>(c)) {
    input_.putback(c);
    return ReadInteger(input_);
  }

  // strings
  if (c == '\'' || c == '"') {
    input_.putback(c);
    return ReadString(input_);
  }

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
  if (checkPolicies<Alpha, CharPolicy<'_'>>(c)) {
    string string_token;
    string_token.push_back(c);
    while (checkPolicies<Alpha, Digit, CharPolicy<'_'>>(char(input_.peek()))) {
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
