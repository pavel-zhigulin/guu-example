#include "token.h"
#include <iostream>

namespace Guu
{

std::ostream& operator<<(std::ostream& os, TokenType tt)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wswitch"

#define CASE(TT) case TokenType::TT: return os << #TT

    switch (tt)
    {
        CASE(SUB);
        CASE(PRINT);
        CASE(SET);
        CASE(CALL);
        CASE(NUM);
        CASE(ID);
        CASE(NEWLINE);
        CASE(SPACE);
        CASE(PLUS);
        CASE(MINUS);
        CASE(EOL);
    }

#pragma GCC diagnostic pop

    return os;
}

std::ostream& operator<<(std::ostream& os, const Token& token)
{
    os << "Token<" << token.type_ << ">";
    switch (token.type_)
    {
        case TokenType::SPACE:
            return os << "(" << (token.value_ == "\t" ? "\\t" : "' '") << ")";
        case TokenType::NEWLINE:
            return os << "(\\n)";
        default:
            return os << "(" << token.value_ << ")";
    }
}

}
