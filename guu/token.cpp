#include <iostream>

#include "token.h"

namespace Guu
{

std::ostream& operator<<(std::ostream& os, TokenType tt)
{
    // clang-format off
    switch(tt)
    {
        #define PRINT_TOKEN_TYPE_NAME(TT, _) case TokenType::TT: return os << #TT;
        GUU_TOKEN_TYPE_VALUES(PRINT_TOKEN_TYPE_NAME)
        #undef PRINT_TOKEN_TYPE_NAME
    }
    // clang-format on

    return os;
}

std::ostream& operator<<(std::ostream& os, const Token& token)
{
    os << "Token<" << token.type_ << ">";
    switch(token.type_)
    {
        case TokenType::SPACE: return os << "(" << (token.value_ == "\t" ? "\\t" : "' '") << ")";
        case TokenType::EOL: return os << "(\\n)";

        default: return os << "(" << token.value_ << ")";
    }
}

}
