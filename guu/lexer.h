#pragma once

#include "token.h"

#include <stack>
#include <string>
#include <optional>

namespace Guu
{

class Tokenizer
{
    using TT       = TokenType;
    using Iterator = std::string::const_iterator;
    using State    = Iterator;

public:
    explicit Tokenizer(std::string text);

    Tokenizer(const Tokenizer&) = delete;
    Tokenizer(Tokenizer&&)      = default;

    Token getNext();

    size_t currentLine() const
    {
        return currLine_;
    }

    bool isEnd() const
    {
        return currentChar_ == std::cend(text_);
    }

    State getState()
    {
        return currentChar_;
    }

    void restoreState(State st)
    {
        currentChar_ = st;
    }

private:
    std::optional<Token> singleCharToken();
    std::string getInteger();
    std::string getId();
    std::string getStringLiteral();

    char step()
    {
        return *(currentChar_++);
    }

private:
    std::string text_;
    Iterator currentChar_;
    size_t currLine_;
    std::stack<Iterator> states_;
};

}
