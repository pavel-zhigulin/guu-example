#include "parser.h"

#include <iostream>
#include <sstream>

namespace Guu
{

std::unique_ptr<AST::Node> Parser::tryParse(std::unique_ptr<AST::Node>(Parser::*memFn)())
{
    auto state = tokenizer_.getState();
    auto savedToken = currToken_;
    try
    {
        return (this->*memFn)();
    }
    catch(...)
    {
        tokenizer_.restoreState(state);
        currToken_ = savedToken;
        return nullptr;
    }
}

/// program ::= statement (NEWLINE+ statement)*
std::unique_ptr<AST::Node> Parser::program()
{
    auto result = std::make_unique<AST::Root>(Token());

    result->children_.push_back( statement() );

    while (!tokenizer_.isEnd())
    {
        eatAll(TokenType::NEWLINE);
        result->children_.push_back(statement());
    }

    eatAll(TokenType::NEWLINE);

    return result;
}

/// SPACE* cmd SPACE*
std::unique_ptr<AST::Node> Parser::statement()
{
    eatAll(TT::SPACE);
    auto result = cmd();
    eatAll(TT::SPACE);

    return result;
}

/// cmd ::= print | call | set | sub
std::unique_ptr<AST::Node> Parser::cmd()
{
    std::unique_ptr<AST::Node> result;

    if     (result = tryParse(&Parser::print); result){}
    else if(result = tryParse(&Parser::call);  result){}
    else if(result = tryParse(&Parser::set);   result){}
    else if(result = tryParse(&Parser::sub);   result){}

    if (!result)
    {
        throw unexpectedToken("cmd");
    }

    return result;
}

/// print ::= PRINT SPACE param
std::unique_ptr<AST::Node> Parser::print()
{
    auto result = std::make_unique<AST::UnaryOp>(currToken_);
    eat(TT::PRINT);
    eat(TT::SPACE);
    result->op_ = param();
    return result;
}

/// call ::= CALL SPACE ID
std::unique_ptr<AST::Node> Parser::call()
{
    auto result = std::make_unique<AST::UnaryOp>(currToken_);
    eat(TT::CALL);
    eat(TT::SPACE);
    result->op_ = std::make_unique<AST::Param>(currToken_);
    eat(TT::ID);

    return result;
}

/// set ::= SET SPACE ID SPACE param
std::unique_ptr<AST::Node> Parser::set()
{
    auto result = std::make_unique<AST::BinOp>(currToken_);
    auto& binop = *result;
    eat(TT::SET);
    eat(TT::SPACE);

    binop.op1_ = std::make_unique<AST::Param>(currToken_);
    eat(TT::ID);

    eat(TT::SPACE);
    binop.op2_ = param();

    return result;
}

/// sub ::= SUB SPACE ID
std::unique_ptr<AST::Node> Parser::sub()
{
    auto result = std::make_unique<AST::ProcDecl>(currToken_);
    eat(TT::SUB);
    eat(TT::SPACE);

    result->name_ = currToken_.value_;
    eat(TT::ID);

    return result;
}

/// param ::= ID | integer
std::unique_ptr<AST::Node> Parser::param()
{
    if (currToken_.type_ == TT::ID)
    {
        auto result = std::make_unique<AST::Param>(currToken_);
        eat(TT::ID);
        return result;
    }
    else
    {
        return integer();
    }
}

/// integer ::= (PLUS | MINUS)? NUM
std::unique_ptr<AST::Node> Parser::integer()
{
    std::string value;
    if (currToken_.type_ == TT::PLUS)
    {
        eat(TT::PLUS);
    }
    else if (currToken_.type_ == TT::MINUS)
    {
        eat(TT::MINUS);
        value.append("-");
    }

    auto tok = currToken_;
    value.append(currToken_.value_);
    eat(TT::NUM);

    return std::make_unique<AST::Param>(tok, value);
}

void Parser::eat(TokenType tt)
{
    if (tt == currToken_.type_)
    {
        currToken_ = tokenizer_.getNext();
    }
    else
    {
        throw unexpectedToken(tt, "eat");
    }
}

void Parser::eatAll(TokenType tt)
{
    while (currToken_.type_ == tt)
        eat(tt);
}

std::runtime_error Parser::unexpectedToken(const char *source)
{
    std::ostringstream ss;
    ss << "Unexpected token in line " << tokenizer_.currentLine()
       << " while parsing '" << source << "'"
       << " [CurrentToken = " << currToken_ << "]";

    return std::runtime_error(ss.str());
}

std::runtime_error Parser::unexpectedToken(TokenType expected, const char *source)
{
    std::ostringstream ss;
    ss << "Unexpected token in line " << tokenizer_.currentLine()
       << " while parsing '" << source << "'"
       << " [Expected = " << expected << ", "
       <<   "Actual = " << currToken_.type_ << "]";

    return std::runtime_error(ss.str());
}

}
