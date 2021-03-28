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
#include <stack>
#include <optional>

/// Guu:
/// ============
/// Where to check:
///     https://mdkrajnak.github.io/ebnftest/
/// Grammar:
///     program   ::= sub (NEWLINE sub)* NEWLINE*
///     sub       ::= NEWLINE* SUB ' ' ID NEWLINE inst_list
///     inst_list ::= (inst | inst NEWLINE+ inst_list)
///     inst      ::= print | call | set
///     print     ::= PRINT ' ' param
///     call      ::= CALL ' ' ID
///     set       ::= SET ' ' ID ' ' param
///     param     ::= ID | NUM
///     PRINT     ::= "print"
///     CALL      ::= "call"
///     SET       ::= "set"
///     SUB       ::= "sub"
///     ID        ::= #"[a-zA-Z0-9]+"
///     NUM       ::= #"[0-9]+"
///     NEWLINE   ::= "\n"


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
    return std::visit([](auto&& t) { return t.type_; }, val);
}

std::string getTokenValue(const TokenVal& val)
{
    return std::visit([](auto&& t) { return t.value_; }, val);
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

template <typename T>
struct IVisitor
{
    virtual void visit(const T&) = 0;
};


template<typename ...T>
struct IVisitorOf : public IVisitor<T>...
{
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

struct Sub
{
    size_t line_;
    Id name_;
    std::vector<Instruction> instructionsList_;
};

struct Program
{
    std::vector<Sub> subs_;
};

struct AstVisitor
    : public IVisitorOf<Program, Sub, Instruction, Print, Call, Set, Param, Integer, Id>
{
public:
    void visit(const Instruction& inst) override
    {
        std::visit([this](const auto& i)
        {
            using VisitorT = IVisitor<std::decay_t<decltype(i)>>;
            static_cast<VisitorT*>(this)->visit(i);
        }, inst);
    }

    void visit(const Param& param) override
    {
        std::visit([this](const auto& p)
        {
            using VisitorT = IVisitor<std::decay_t<decltype(p)>>;
            static_cast<VisitorT*>(this)->visit(p);
        }, param);
    }

    void visit(const Integer&) override
    {}

    void visit(const Id&) override
    {}

    void visit(const Call&) override
    {}

    void visit(const Set&) override
    {}

    void visit(const Print&) override
    {}

    void visit(const Sub&) override
    {}

    void visit(const Program&) override
    {}
};


class AstPrinter
    : public AstVisitor
{
    static constexpr int INDENT_STEP = 2;

public:
    template <typename AstNode>
    void print(std::ostream& os, AstNode&& node)
    {
        os_ = os;
        indent_ = 0;
        visit(node);
    }

private:
    using AstVisitor::visit;

    std::ostream& os()
    {
        return os_.value().get();
    }

    void visit(const Program& program) override
    {
        indent();
        os() << "(Program)" << std::endl;

        addIndent();
        for(auto&& s : program.subs_)
        {
            visit(s);
        }
        subIndent();
    }

    void visit(const Sub& sub) override
    {
        indent();
        os() << "(Sub line=" << sub.line_ << ")" << std::endl;

        addIndent();
        visit(sub.name_);
        for(auto&& i : sub.instructionsList_)
        {
            visit(i);
        }
        subIndent();
    }

    void visit(const Print& print) override
    {
        indent();
        os() << "(Print)" << std::endl;
        addIndent();
        visit(print.param_);
        subIndent();
    }

    void visit(const Call& call) override
    {
        indent();
        os() << "(Call)" << std::endl;
        addIndent();
        visit(call.id_);
        subIndent();
    }

    void visit(const Set& set) override
    {
        indent();
        os() << "(Set)" << std::endl;
        addIndent();
        visit(set.id_);
        visit(set.param_);
        subIndent();
    }

    void visit(const Integer& integer) override
    {
        indent();
        os() << "(Integer<" << integer << ">)" << std::endl;
    }

    void visit(const Id& id) override
    {
        indent();
        os() << "(Id<" << id << ">)" << std::endl;
    }

private:
    void indent()
    {
        os() << std::string(indent_, ' ');
    }

    void addIndent()
    {
        indent_ += INDENT_STEP;
    }

    void subIndent()
    {
        indent_ -= INDENT_STEP;
    }

private:
    using Ref = std::reference_wrapper<std::ostream>;
    std::optional<Ref> os_ = std::nullopt;
    int indent_ = 0;
};

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

    /// inst_list ::= (inst | inst ('\n')+ inst_list)
    std::vector<Instruction> instructionsList()
    {
        std::vector<Instruction> result;

        auto inst = instruction();

        if (not inst)
        {
            throw this->instructionExpected();
        }

        result.emplace_back( *inst );

        while (tokenHasType<TokenType::NEWLINE>(currToken_))
        {
            eat<TokenType::NEWLINE>();

            auto inst = instruction();

            if (not inst)
            {
                //It is fine, because we reach end of the instrument_list
                break;
            }

            result.emplace_back(std::move(*inst));
        }

        return result;
    }

    /// inst ::= print | call | set
    std::optional<Instruction> instruction()
    {
        std::optional<Instruction> result = {};

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
                        result = std::nullopt;
                    }
                },
                currToken_
        );

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
            throw unexpectedToken<TT>();
        }
    }

    template <TokenType TT>
    std::runtime_error  unexpectedToken()
    {
        std::ostringstream ss;
        ss << "Unexpected token in line " << tokenizer_.currentLine()
           << " [Expected = " << Token<TT>("") << ", "
           <<   "Actual = " << currToken_ << "]";

        return std::runtime_error(ss.str());
    }

    std::runtime_error instructionExpected()
    {
        std::ostringstream ss;
        ss << "Instruction token expected in line " << tokenizer_.currentLine()
           << ", but got ";

        std::visit([&](auto&& t){ ss << t; }, currToken_);

        return std::runtime_error(ss.str());
    }


    Tokenizer tokenizer_;
    TokenVal currToken_;
    TokenVal prevToken_;
};


class Interpreter
    : public AstVisitor
{
    struct SymTable
    {
        size_t line_;
        Id subName_;
    };
    struct Ret
    {
        const Ret& operator*() const { return *this; }
    };


public:
    Interpreter(Program prg)
        : prg_(prg)
    {
        compile();
        currentInstruction_ = std::cbegin(text_["main"]);

        text_["__eof"].push_back(Ret{});
        auto temp = std::cbegin(text_["__eof"]);
        stack_.push(temp);
        symbolsTable_[&(*temp)] = {};
    }

    bool isEnd()
    {
        return finished_;
    }

    void stepInto()
    {
        if (isEnd())
        {
            return;
        }

        std::visit(
            [this](auto&& inst){ execute(*inst); },
            *currentInstruction_
        );
    }

    void stepOver()
    {
        while (std::holds_alternative<Ret>(*currentInstruction_) && !isEnd())
        {
            stepInto();
        }

        if (isEnd())
        {
            return;
        }

        auto next = currentInstruction_+1;
        while (currentInstruction_ != next)
        {
            stepInto();
        }
    }

    void printVars()
    {
        std::cout << std::endl << "VARIABLES: " << std::endl;
        for(auto&& [k, v] : globalMemory_)
        {
            std::cout << "    " << k << " = " << v << std::endl;
        }
        std::cout << std::endl;
    }

    void bt()
    {
        std::cout << std::endl << "Backtrace:" << std::endl;

        auto backTrace = stack_;
        size_t num = 0;

        auto symTable = symbolsTable_.at(&(*currentInstruction_));
        std::cout << "    #" << num << " " << symTable.subName_ << "() at " << (symTable.line_) << std::endl;

        num++;
        while (backTrace.size() != 1)
        {
            const auto* addr = &(*backTrace.top());
            auto symTable = symbolsTable_.at(addr);
            backTrace.pop();

            std::cout << "    #" << num << " " << symTable.subName_ << "() at " << (symTable.line_) << std::endl;
            num++;
        }
        std::cout << std::endl;
    }

private:
    void execute(const Call& call)
    {
        stack_.push(currentInstruction_+1);
        currentInstruction_ = std::cbegin(text_[call.id_]);
    }

    void execute(const Print& print)
    {
        std::visit(
            overloaded{
                [](Integer i){ std::cout << i << std::endl; },
                [this](Id id){ std::cout << getVariable(id) << std::endl; }
            },
            print.param_
        );
        currentInstruction_++;
    }

    void execute(const Set& set)
    {
        std::visit(
            overloaded{
                [&](Integer i){ setVariable(set.id_, i);  },
                [&](Id id){ setVariable(set.id_, getVariable(id)); }
            },
            set.param_
        );
        currentInstruction_++;
    }

    void execute(const Ret& ret)
    {
        currentInstruction_ = stack_.top();
        stack_.pop();
        if (stack_.size() == 0)
        {
            finished_ = true;
        }
    }


private:
    void compile()
    {
        visit(prg_);
    }

    using AstVisitor::visit;

    void visit(const Program& program) override
    {
        for(auto&& s : program.subs_)
        {
            visit(s);
        }
    }

    void visit(const Sub& sub) override
    {
        for(auto&& i : sub.instructionsList_)
        {
            std::visit(
                [&](const auto& v)
                {
                    text_[sub.name_].push_back(&v);
                },
                i
            );
        }
        text_[sub.name_].push_back(Ret{});

        for(const auto& i : text_[sub.name_])
        {
            symbolsTable_[&i] = SymTable{ sub.line_, sub.name_ };
        }
    }

private:
    Integer getVariable(Id id)
    {
        if (not globalMemory_.count(id))
        {
            throw std::runtime_error("Unknown variable id='" + id + "'. Stop.");
        }

        return globalMemory_.at(id);
    }

    void setVariable(Id id, Integer value)
    {
        globalMemory_[id] = value;
    }


private:
    Program prg_;

    std::unordered_map<Id, Integer> globalMemory_ = {};

    using Code = std::variant<const Call*, const Set*, const Print*, Ret>;

    std::unordered_map<Id, std::vector<Code>> text_ = {};
    std::unordered_map<const Code*, SymTable> symbolsTable_;

    using InstIter = decltype(decltype(text_)::value_type::second)::const_iterator;
    InstIter currentInstruction_;

    bool finished_ = false;

    std::stack<InstIter> stack_;
};

int main()
{
    const std::string text =
            "sub foo\n"
            "set a 1\n"
            "sub main\n\n\n"
            "call foo\n"
            "print a\n"
            "set a 2\n"
            "set b a\n"
            "call walle\n"
            "sub walle\n"
            "print a\n"
            "print b\n"
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

        AstPrinter printer;
        printer.print(std::cout, res);

        Interpreter interpreter(res);

        interpreter.stepInto();
        interpreter.bt();
        interpreter.stepOver();
        interpreter.stepOver();
        interpreter.stepOver();
        interpreter.stepOver();
        interpreter.printVars();
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }


    return 0;
}
