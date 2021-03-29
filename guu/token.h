#pragma once

#include <string>
#include <iosfwd>

namespace Guu
{

#define TOKEN_TYPES(_) \
    _(SUB) \
    _(PRINT) \
    _(SET) \
    _(CALL) \
    _(NUM) \
    _(ID) \
    _(NEWLINE) \
    _(SPACE) \
    _(PLUS) \
    _(MINUS) \
    _(EOL) \

///     SUB        ::= "sub"
///     PRINT      ::= "print"
///     SET        ::= "set"
///     CALL       ::= "call"
///     NUM        ::= #"[0-9]+"
///     ID         ::= #"[a-zA-Z][a-zA-z0-9]+"
///     NEWLINE    ::= '\n'
///     SPACE      ::= ' '
///     PLUS       ::= '+'
///     MINUS      ::= '-'

#define RIGHT_COMMA(x) x,
enum class TokenType
{
    TOKEN_TYPES(RIGHT_COMMA)
};
#undef RIGHT_COMMA

std::ostream& operator<<(std::ostream& os, TokenType tt);

struct Token
{
    Token(TokenType tt, std::string s) : type_(tt), value_(std::move(s)) {}
    Token(TokenType tt) : Token(tt, std::string("")) {};
    Token(TokenType tt, char c) : Token(tt, std::string(1, c)){}
    Token() : Token(TokenType::EOL, ""){}

    Token(const Token&) = default;
    Token(Token&&) = default;
    Token& operator=(const Token&) = default;
    Token& operator=(Token&&) = default;

    TokenType type_;
    std::string value_;
};
std::ostream& operator<<(std::ostream& os, const Token& token);

#ifdef SAVE_TOKEN_TYPES
#undef SAVE_TOKEN_TYPES
#else
#undef TOKEN_TYPES
#endif

}


