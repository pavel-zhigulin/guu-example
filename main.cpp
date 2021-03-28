#include <iostream>
#include <variant>
#include <type_traits>
#include <cctype>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <charconv>
#include <sstream>
#include <set>
#include <optional>

/// Guu:
/// ============
/// Where to check:
///     https://mdkrajnak.github.io/ebnftest/
/// Grammar:
///     program ::= sub ('\n' (sub))*
///     sub ::= ('\n')* SUB ' ' ID '\n' inst_list
///     inst_list ::= (inst | inst ('\n')+ inst_list)
///     inst ::= print | call | set
///     print ::= PRINT ' ' param
///     call ::= CALL ' ' ID
///     set ::= SET ' ' ID ' ' param
///     param ::= ID | NUM
///     PRINT ::= "print"
///     CALL ::= "call"
///     SET ::= "set"
///     SUB ::= "sub"
///     ID ::= #"[a-zA-Z0-9]+"
///     NUM ::= #"[0-9]+"


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

template<TokenType TT>
Token<TT> get(const TokenVal& val)
{
    return std::get< Token<TT> >(val);
}

TokenType getTokenType(const TokenVal& val)
{
    return std::visit([&](auto&& t) { return t.type_; }, val);
}

std::string getTokenValue(const TokenVal& val)
{
    return std::visit([&](auto&& t) { return t.value_; }, val);
}

template<TokenType ...TT>
bool tokenHasType(const TokenVal& val)
{
    return (std::holds_alternative<Token<TT>>(val) || ...);
}


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

std::ostream& operator<<(std::ostream& os, const TokenVal& val)
{
    std::visit([&os](auto&& token){ os << token; }, val);
    return os;
}


class Tokenizer
{
public:
    Tokenizer(std::string text)
            : text_(std::move(text))
            , currentChar_(text_.begin())
            , currLine_(1)
    {

    }

    Tokenizer(Tokenizer&& other)
        : text_(std::string())
        , currentChar_(std::cend(text_))
        , currLine_(static_cast<size_t>(-1))
    {
        using std::swap;

        if (other.isEnd())
        {
            swap(text_, other.text_);
            swap(currLine_, other.currLine_);
            currentChar_ = std::cend(text_);
        }
        else
        {
            auto pos = std::distance(std::cbegin(other.text_), other.currentChar_);
            swap(text_, other.text_);
            swap(currLine_, other.currLine_);
            currentChar_ = std::cbegin(text_) + pos;
        }

    }


    TokenVal getNext()
    {
        skipSpaces();

        if (*currentChar_ == '\n')
        {
            skipNewLines();
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

        if(std::isalnum(*currentChar_))
        {
            std::string tv = advance();

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

            return construct<TokenType::ID>(tv);
        }

        throw std::runtime_error("Unexpected symbol '" + std::string(1, *currentChar_) + "' on line " + std::to_string(currLine_));
    }

    TokenVal peekNext()
    {
        auto prevPos = currentChar_;
        auto result = getNext();
        currentChar_ = prevPos;
        return result;
    }

    size_t currentLine() const
    {
        return currLine_;
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
        while (!isEnd() && std::isspace(*currentChar_) && *currentChar_ != '\n')
        {
            currentChar_++;
        }
    }

    void skipNewLines()
    {
        while (*currentChar_ == '\n')
        {
            currentChar_++;
            currLine_++;
            skipSpaces();
        }
    }

    bool isEnd() const
    {
        return currentChar_ == std::cend(text_);
    }

    template<TokenType TT>
    TokenVal construct(std::string value)
    {
        return Token<TT>{std::move(value)};
    }

    std::optional<char> peek()
    {
        return (currentChar_+1) != std::cend(text_)
               ? std::optional<char>{*(currentChar_+1)}
               : std::nullopt;
    }

private:
    using Iterator = decltype(std::declval<std::string>().cbegin());

    std::string text_;
    Iterator currentChar_;
    size_t currLine_;
};

using Id = std::string;
using Integer = std::int64_t;

using Param = std::variant<Id, Integer>;

struct Set
{
    Id id_;
    Param param_;
};

struct Call
{
    Id id_;
};

struct Print
{
    Param param_;
};

using Instruction = std::variant<Set, Call, Print>;

void print(const Instruction& i, std::ostream& os, int indent)
{
    auto printParam = [&](auto&& p){ os << p; };

    os << std::string(indent, ' ');
    std::visit(
            overloaded{
                [&](const Set& s)
                {
                    os << "(Set id=" << s.id_ << " param=";
                    std::visit(printParam, s.param_);
                    os << ")";
                },
                [&](const Call& c)
                {
                    os << "(Call id=" << c.id_ << ")";
                },
                [&](const Print& p)
                {
                    os << "(Print param=";
                    std::visit(printParam, p.param_);
                    os << ")";
                }
            },
            i
    );
    os << std::endl;
}

struct Sub
{
    size_t line_;
    std::string name_;
    std::vector<Instruction> instructionsList_;
};

void print(const Sub& s, std::ostream& os, int indent)
{
    std::cout << std::string(indent, ' ') <<  "(Sub line=" << s.line_ << " name=" << s.name_ << ")" << std::endl;
    for(auto&& i : s.instructionsList_)
    {
        print(i, os, indent+2);
    }
}

struct Program
{
    std::vector<Sub> subs_;
};

void printAst(const Program& prg)
{
    std::cout << "(Program)" << std::endl;

    for(auto&& s : prg.subs_)
    {
        print(s, std::cout, 2);
    }
}

class Parser
{
public:
    Parser(Tokenizer t)
        : tokenizer_(std::move(t))
        , currToken_(tokenizer_.getNext())
        , prevToken_(currToken_)
    {
    }

    Program buildAST()
    {
        return program();
    }

    /// program ::= sub ('\n' (sub))*
    Program program()
    {
        Program result;

        result.subs_.emplace_back( sub() );

        while( not tokenHasType<TokenType::EOL>(currToken_) )
        {
            result.subs_.emplace_back( sub() );
        }

        return result;
    }

    /// sub ::= ('\n')* SUB ' ' ID '\n' inst_list
    Sub sub()
    {
        while (tokenHasType<TokenType::NEWLINE>(currToken_))
        {
            eat<TokenType::NEWLINE>();
        }

        eat<TokenType::SUB>();

        Sub result;
        result.line_ = tokenizer_.currentLine();
        result.name_ = id();

        eat<TokenType::NEWLINE>();
        result.instructionsList_ = instructionsList();

        return result;
    }

    ///     inst_list ::= (inst | inst ('\n')+ inst_list)
    std::vector<Instruction> instructionsList()
    {
        std::vector<Instruction> result;

        result.emplace_back( instruction() );

        while (tokenHasType<TokenType::NEWLINE>(currToken_))
        {
            eat<TokenType::NEWLINE>();

            if (not isInstruction(currToken_))
            {
                break;
            }

            result.emplace_back( instruction() );
        }

        return result;
    }

    /// inst ::= print | call | set
    Instruction instruction()
    {
        Instruction result;

        std::visit(
                overloaded {
                    [&](Token<TokenType::PRINT>&)
                    {
                        eat<TokenType::PRINT>();
                        result = Print{ param() };
                    },
                    [&](Token<TokenType::CALL>&)
                    {
                        eat<TokenType::CALL>();
                        result = Call{ id() };
                    },
                    [&](Token<TokenType::SET>&)
                    {
                        eat<TokenType::SET>();
                        result = Set{ id(), param() };
                    },
                    [&](auto&& token)
                    {
                        instructionExpected(token);
                    }
                },
                currToken_);
        return result;
    }

    /// param ::= ID | NUM
    Param param()
    {
        Param result;
        std::visit(
                overloaded{
                    [&](Token<TokenType::ID>& token)
                    {
                        result = id();
                    },
                    [&](Token<TokenType::INTEGER>& token)
                    {
                        result = integer();
                    },
                    [&](auto&& token)
                    {
                        throw std::runtime_error("unexpected token " + token.value_);
                    }
                },
                currToken_
        );

        return result;
    }

    /// NUM ::= #"[0-9]+"
    Integer integer()
    {
        Integer result;
        auto token = get<TokenType::INTEGER>(currToken_);
        currToken_ = tokenizer_.getNext();

        auto [p, ec] =  std::from_chars(
                            token.value_.data(),
                            token.value_.data() + token.value_.size(),
                            result
                        );

        if(ec != std::errc())
        {
            throw std::runtime_error("Cannot cast integer");
        }

        return result;
    }

    // ID ::= #"[a-zA-Z0-9]+"
    Id id()
    {
        Id value = get<TokenType::ID>(currToken_).value_;
        currToken_ = tokenizer_.getNext();
        return value;
    }

private:
    bool isInstruction(const TokenVal& val)
    {
        return tokenHasType<TokenType::PRINT, TokenType::CALL, TokenType::SET>(currToken_);
    }

    template<TokenType TT>
    void eat()
    {
        if (tokenHasType<TT>(currToken_) )
        {
            currToken_ = tokenizer_.getNext();
        }
        else
        {
            unexpectedToken<TT>();
        }
    }

    template <TokenType TT>
    void unexpectedToken()
    {
        std::ostringstream ss;
        ss << "Unexpected token in line " << tokenizer_.currentLine()
           << " [Expected = " << Token<TT>("") << ", "
           <<   "Actual = " << currToken_ << "]";

        throw std::runtime_error(ss.str());
    }

    template <TokenType TT>
    void instructionExpected(const Token<TT>& token)
    {
        std::ostringstream ss;
        ss << "Instruction token expected in line " << tokenizer_.currentLine()
           << ", but got " << token;

        throw std::runtime_error(ss.str());
    }


    Tokenizer tokenizer_;
    TokenVal currToken_;
    TokenVal prevToken_;
};


int main()
{
    const std::string text =
            "sub foo\n"
            "set a 1\n"
            "sub main\n\n\n"
            "set a 1\n"
            "print a\n"
            "call main\n"
            "sub walle\n"
            "print a"
            ;

    try
    {
        std::cout << "PROGRAM:" << std::endl;
        std::cout << text << std::endl << std::endl;

        std::cout << "TOKENS:" << std::endl;
        auto t = Tokenizer(text);
        auto tv = t.getNext();
        while(getTokenType(tv) != TokenType::EOL)
        {
            std::cout << tv << std::endl;
            tv = t.getNext();
        }
        std::cout << std::endl;

        std::cout << "PARSER:" << std::endl;
        auto p = Parser(Tokenizer(text));
        auto res = p.buildAST();
        printAst(res);
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }


    return 0;
}
