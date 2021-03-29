#pragma once

#include <string>
#include <iosfwd>

namespace Guu
{

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
enum class TokenType
{
    SUB,
    PRINT,
    SET,
    CALL,
    NUM,
    ID,
    NEWLINE,
    SPACE,
    PLUS,
    MINUS,
    EOL
};
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

}

