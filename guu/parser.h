#pragma once

#include "lexer.h"
#include "ast.h"

#include <iosfwd>
#include <stdexcept>

namespace Guu
{
using ValidationSource = const char*;

class Parser
{
    using TT = TokenType;

    enum class EatSpaces
    {
        Left,
        Right,
        Both,
    };

public:
    Parser(Tokenizer t) : tokenizer_(std::move(t)), currToken_(tokenizer_.getNext())
    {
    }

    AST::Node::Ptr buildAST()
    {
        return program();
    }

private:
    AST::Node::Ptr program();
    AST::Node::Ptr fn();
    AST::Node::Ptr fn_arg();
    AST::Node::Ptr statement();
    AST::Node::Ptr type_id();

private:
    template <typename Node, typename... Args>
    auto construct(Args... args)
    {
        return std::make_unique<Node>(std::forward<Args>(args)...);
    }

    AST::Node::Ptr tryParse(AST::Node::Ptr (Parser::*memFn)());

    template <typename... Args>
    void eat(TokenType tt, Args... rest)
    {
        eat(tt);
        eat(rest...);
    }

    template <typename... Args>
    void eatWithSpaces(TokenType tt, Args... rest)
    {
        eatWithSpaces(tt, EatSpaces::Left);
        eatWithSpaces(rest...);
    }

    void eatEmptyLines();
    void eat(TokenType tt);
    void eatAll(TokenType tt);
    void eatWithSpaces(TokenType tt, EatSpaces policy = EatSpaces::Left);

    std::string eatVal(TokenType tt);
    std::string eatValueWithSpaces(TokenType tt, EatSpaces policy = EatSpaces::Left);

    void checkTokenType(TokenType tt, ValidationSource source);
    void checkTokenValue(std::string value, ValidationSource source);

    std::runtime_error unexpectedValue(std::string expected, ValidationSource source);
    std::runtime_error unexpectedToken(ValidationSource source);
    std::runtime_error unexpectedToken(TokenType expected, ValidationSource source);

private:
    Tokenizer tokenizer_;
    Token currToken_;
};
}