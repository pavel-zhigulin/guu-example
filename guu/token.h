#pragma once

#include <string>
#include <iosfwd>

namespace Guu
{

#define GUU_TOKEN_TYPE_VALUES(_)                                       \
    _(EOL, "EOL ::= '\\n'")                                            \
    _(SPACE, "SPACE ::= ' '")                                          \
    _(COLON, "COLON ::= ':'")                                          \
    _(SEMICOLON, "SEMICOLON ::= ';'")                                  \
    _(COMMA, "COMMA ::= ','")                                          \
    _(MINUS, "MINUS ::= '-'")                                          \
    _(EQ, "EQ ::= '='")                                                \
    _(GT, "GT ::= '>'")                                                \
    _(O_BRACE, "O_BRACE ::= '{'")                                      \
    _(C_BRACE, "C_BRACE ::= '}'")                                      \
    _(O_BRACK, "O_BRACK ::= '['")                                      \
    _(C_BRACK, "C_BRACK ::= ']'")                                      \
    _(O_PAREN, "O_PAREN ::= '('")                                      \
    _(C_PAREN, "C_PAREN ::= ')'")                                      \
    _(NUM, "NUM ::= #'[0-9]+'")                                        \
    _(ESC_SEQ, "ESC_SEQ ::= #'\\\\[a-z\\'\\\"\\\\]'")                  \
    _(ID, "ID ::= #'[a-zA-Z][_a-zA-Z0-9]*'")                           \
    _(STRING_LITERAL, "STRING_LITERAL ::= <just look at guu.grammar>") \
    _(END, "END ::= EOF")

// clang-format off
enum class TokenType : std::uint32_t
{
    #define MAKE_ENUM(name, _) name,
    GUU_TOKEN_TYPE_VALUES(MAKE_ENUM)
    #undef MAKE_ENUM
};
// clang-format on

std::ostream& operator<<(std::ostream& os, TokenType tt);

struct Token
{
    Token(TokenType tt, std::string s) : type_(tt), value_(std::move(s))
    {
    }

    Token(TokenType tt) : Token(tt, std::string("")){};

    Token(TokenType tt, char c) : Token(tt, std::string(1, c))
    {
    }

    Token() : Token(TokenType::END, "")
    {
    }

    Token(const Token&)            = default;
    Token(Token&&)                 = default;
    Token& operator=(const Token&) = default;
    Token& operator=(Token&&)      = default;

    TokenType type_;
    std::string value_;
};

std::ostream& operator<<(std::ostream& os, const Token& token);

}
