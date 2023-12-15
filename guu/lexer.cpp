#include "lexer.h"

#include <stdexcept>
#include <algorithm>
#include <map>
#include <cassert>

namespace Guu
{

Tokenizer::Tokenizer(std::string text) : text_(std::move(text))
{
    currentChar_ = std::cbegin(text_);
    currLine_    = 1;
    while(*currentChar_ == '\n' && !isEnd())
    {
        currentChar_++;
        currLine_++;
    }
}

Token Tokenizer::getNext()
{
    if(isEnd())
        return Token(TT::END);

    if(std::isspace(*currentChar_) && *currentChar_ != '\n')
        return Token(TT::SPACE, step());

    if(auto token = singleCharToken(); token)
    {
        if(token->type_ == TT::EOL)
            ++currLine_;

        return *token;
    }

    if(*currentChar_ == '"' || *currentChar_ == '\'')
        return Token(TT::STRING_LITERAL, getStringLiteral());

    if(std::isdigit(*currentChar_))
        return Token{TT::NUM, getInteger()};

    if(std::isalpha(*currentChar_))
    {
        std::string tv = getId();
        return Token{TT::ID, tv};
    }

    throw std::runtime_error("Unexpected symbol '" + std::to_string(*currentChar_) + "' on line "
                             + std::to_string(currLine_));
}

std::optional<Token> Tokenizer::singleCharToken()
{
    static const std::map<char, TT> charTT = {
        {'\n',       TT::EOL},
        { ':',     TT::COLON},
        { ';', TT::SEMICOLON},
        { ',',     TT::COMMA},
        { '{',   TT::O_BRACE},
        { '}',   TT::C_BRACE},
        { '[',   TT::O_BRACK},
        { ']',   TT::C_BRACK},
        { '(',   TT::O_PAREN},
        { ')',   TT::C_PAREN},
        { '-',     TT::MINUS},
        { '>',        TT::GT},
        { '=',        TT::EQ},
    };

    auto res =
        std::find_if(std::cbegin(charTT), std::cend(charTT), [this](auto& it) { return it.first == *currentChar_; });

    if(res != std::cend(charTT))
        return Token(res->second, step());

    return std::nullopt;
}

std::string Tokenizer::getStringLiteral()
{
    char quot = step();
    auto beg  = currentChar_;

    assert(quot == '"' || quot == '\'');

    while(!isEnd())
    {
        if(*currentChar_ == quot && *(currentChar_ - 1) != '\\')
        {
            step();
            return std::string{beg, currentChar_ - 1};
        }

        step();
    }

    throw std::runtime_error("String literal is not closed on line " + std::to_string(currLine_));
}

std::string Tokenizer::getInteger()
{
    auto begin   = currentChar_;
    auto end     = std::find_if_not(currentChar_, std::cend(text_), ::isdigit);
    currentChar_ = end;

    return std::string{begin, end};
}

std::string Tokenizer::getId()
{
    auto pred    = [](char c) { return c == '_' || ::isalnum(c); };
    auto begin   = currentChar_;
    auto end     = std::find_if_not(currentChar_, std::cend(text_), pred);
    currentChar_ = end;

    return std::string{begin, end};
}

}
