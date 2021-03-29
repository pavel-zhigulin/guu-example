#include "lexer.h"

#include <stdexcept>
#include <algorithm>

namespace Guu
{

Tokenizer::Tokenizer(std::string text)
    : text_(std::move(text))
{
    currentChar_ = std::cbegin(text_);
    currLine_ = 1;
    while(*currentChar_ == '\n' && !isEnd())
    {
        currentChar_++;
        currLine_++;
    }
}

Token Tokenizer::getNext()
{
    if (isEnd())
    {
        return Token{TT::EOL};
    }

    if (std::isspace(*currentChar_) && *currentChar_ != '\n')
    {
        return Token{TT::SPACE, step()};
    }

    if (*currentChar_ == '\n')
    {
        currLine_++;
        return Token{TT::NEWLINE, step()};
    }

    if (*currentChar_ == '+')
    {
        return Token{TT::PLUS, step()};
    }

    if (*currentChar_ == '-')
    {
        return Token{TT::MINUS, step()};
    }

    if (std::isdigit(*currentChar_))
    {
        return Token{TT::NUM, getInteger()};
    }

    if(std::isalpha(*currentChar_))
    {
        std::string tv = advance();

        if(tv == "sub")
        {
            return Token{TT::SUB, tv};
        }
        if (tv == "print")
        {
            return Token{TT::PRINT, tv};
        }
        if (tv == "call")
        {
            return Token{TT::CALL, tv};
        }
        if(tv == "set")
        {
            return Token{TT::SET, tv};
        }

        return Token{TT::ID, tv};
    }

    throw std::runtime_error("Unexpected symbol '" + std::to_string(*currentChar_) + "' on line " + std::to_string(currLine_));
}

std::string Tokenizer::getInteger()
{
    auto begin = currentChar_;
    auto end = std::find_if_not(currentChar_, std::cend(text_), ::isdigit);
    currentChar_ = end;

    return std::string{begin, end};
}

std::string Tokenizer::advance()
{
    auto begin = currentChar_;
    auto end = std::find_if_not(currentChar_, std::cend(text_), ::isalnum);
    currentChar_ = end;

    return std::string{begin, end};
}

}
