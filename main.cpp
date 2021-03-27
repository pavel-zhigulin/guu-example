#include <iostream>
#include <variant>
#include <type_traits>
#include <cctype>
#include <algorithm>
#include <stdexcept>
#include <set>

/// Guu grammar:
/// ============
///     program: sub (sub)*
///     sub: SUB ID instructions_list NEWLINE
///     instructions_list: NEWLINE instruction (NEWLINE instruction)*
///     instruction: print | set | call
///     print: PRINT param
///     set: SET ID param
///     call: CALL ID
///     param: ID | INTEGER

// Just helper for std::visit, took from example https://en.cppreference.com/w/cpp/utility/variant/visit
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

using namespace std::string_literals;

enum class TokenType
{
    SUB,
    ID,
    PRINT,
    CALL,
    NEWLINE,
    SET,
    INTEGER,
    EOL
};

std::ostream &operator<<(std::ostream& os, TokenType tt)
{
    switch (tt)
    {
        case TokenType::SUB:     return os << "SUB";
        case TokenType::ID:      return os << "ID";
        case TokenType::PRINT:   return os << "PRINT";
        case TokenType::CALL:    return os << "CALL";
        case TokenType::NEWLINE: return os << "NEWLINE";
        case TokenType::SET:     return os << "SET";
        case TokenType::INTEGER: return os << "INTEGER";
        case TokenType::EOL:     return os << "EOL";
    }
    return os;
}

template<TokenType TT>
struct Token
{
    Token(std::string v)
            : value_(std::move(v))
    {}

    TokenType type_ = TT;
    std::string value_;
};

template<TokenType TT>
std::ostream& operator<<(std::ostream& os, const Token<TT>& token)
{
    return os << "Token<" << token.type_ << ">(" << token.value_ << ")";
}

template<>
std::ostream& operator<<(std::ostream& os, const Token<TokenType::NEWLINE>& token)
{
    return os << "Token<" << token.type_ << ">(\\n)";
}


class Tokenizer
{
public:
    using TokenVal =
    std::variant<
            Token<TokenType::SUB>,
            Token<TokenType::ID>,
            Token<TokenType::PRINT>,
            Token<TokenType::CALL>,
            Token<TokenType::NEWLINE>,
            Token<TokenType::SET>,
            Token<TokenType::INTEGER>,
            Token<TokenType::EOL>
    >;

    Tokenizer(std::string text)
            : text_(std::move(text))
            , currentChar_(text_.begin())
            , prevTokenType_(TokenType::NEWLINE)
            , currLine_(1)
    {}

    TokenVal getNext()
    {
        skipSpaces();

        if (*currentChar_ == '\n')
        {
            currLine_++;
            currentChar_++;
            return construct<TokenType::NEWLINE>("\n");
        }

        if (isEnd())
        {
            return construct<TokenType::EOL>(std::string());
        }

        if (std::isdigit(*currentChar_))
        {
            return construct<TokenType::INTEGER>(getInteger());
        }

        if(std::isalpha(*currentChar_))
        {
            std::string tv = advance();

            if (prevTokenType_ == TokenType::NEWLINE)
            {
                if(tv == "sub")
                {
                    return construct<TokenType::SUB>(tv);
                }
                if (tv == "print")
                {
                    return construct<TokenType::PRINT>(tv);
                }
                if(tv == "call")
                {
                    return construct<TokenType::CALL>(tv);
                }
                if(tv == "set")
                {
                    return construct<TokenType::SET>(tv);
                }
                throw std::runtime_error("Unexpected token '" + tv + "' on line " + std::to_string(currLine_));
            }

            return construct<TokenType::ID>(tv);
        }
    }

private:
    std::string getInteger()
    {
        auto begin = currentChar_;
        auto end =
                std::find_if_not(
                        currentChar_,
                        std::cend(text_),
                        [](char v) { return std::isdigit(v); }
                );

        currentChar_ = end;

        if (not isEnd() && not std::isspace(*currentChar_))
        {
            throw std::runtime_error("invalid integer");
        }

        return std::string(begin, end);
    }

    std::string advance()
    {
        auto begin = currentChar_;
        auto end =
                std::find_if_not(
                        currentChar_,
                        std::cend(text_),
                        [](char v) { return std::isalnum(v); }
                );

        currentChar_ = end;

        return std::string(begin, end);
    }

    void skipSpaces()
    {
        while (std::isspace(*currentChar_) && *currentChar_ != '\n' && !isEnd())
        {
            currentChar_++;
        }
    }

    bool isEnd()
    {
        return currentChar_ == std::cend(text_);
    }

    template<TokenType TT>
    TokenVal construct(std::string value)
    {
        prevTokenType_ = TT;
        return Token<TT>{std::move(value)};
    }

private:
    using Iterator = decltype(std::declval<std::string>().cbegin());

    TokenType prevTokenType_;
    std::string text_;
    Iterator currentChar_;
    size_t currLine_;
};

int main()
{
    const std::string text =
            "sub main\n"
            "  set a 1\n"
            "print a\n"
            "call main\n"
            ;

    Tokenizer t(text);

    bool end = false;
    while (!end)
    {
        try
        {
            auto val = t.getNext();
            std::visit(
                    overloaded{
                            [&](Token<TokenType::EOL> token)
                            {
                                std::cout << token << std::endl;
                                end = true;
                            },
                            [](auto &&token)
                            {
                                std::cout << token << std::endl;
                            }
                    }, val
            );
        }
        catch(const std::runtime_error& e)
        {
            std::cerr << "ERROR: " << e.what() << std::endl;
            break;
        }
    }

    return 0;
}
