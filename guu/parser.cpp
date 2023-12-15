#include "parser.h"

#include <iostream>
#include <sstream>
#include <cassert>

#define UNEXPECTED_VAL(expected) unexpectedValue(expected, __PRETTY_FUNCTION__)

namespace Guu
{

AST::Node::Ptr Parser::tryParse(AST::Node::Ptr (Parser::*memFn)())
{
    auto state      = tokenizer_.getState();
    auto savedToken = currToken_;
    try
    {
        return (this->*memFn)();
    } catch(...)
    {
        tokenizer_.restoreState(state);
        currToken_ = savedToken;
        return nullptr;
    }
}

// program ::= (fn eol*)+
AST::Node::Ptr Parser::program()
{
    auto result = construct<AST::Root>();

    eatEmptyLines();
    result->children_.push_back(fn());

    // while(!tokenizer_.isEnd())
    // {
    //     eatAll(TokenType::NEWLINE);
    //     result->children_.push_back(statement());
    // }

    eatEmptyLines();

    assert(currToken_.type_ == TT::END);
    return result;
}

// fn ::= "fn" SPACE spaces fn_name fn_args fn_ret o_brace fn_content c_brace
AST::Node::Ptr Parser::fn()
{
    // "fn"
    std::string fnStr = eatVal(TT::ID);
    if(fnStr != "fn")
        throw UNEXPECTED_VAL("fn");

    // SPACE
    eat(TT::SPACE);

    // fn_name ::= id
    std::string id = eatValueWithSpaces(TT::ID);

    // fn_args ::= o_paren (spaces | fn_arg (comma fn_arg)*) c_paren
    eatWithSpaces(TT::O_PAREN);

    AST::Node::Ptr arg = nullptr;
    AST::NodeVec fnArgs;
    while((arg = fn_arg()))
    {
        fnArgs.emplace_back(std::move(arg));
    }

    eatValueWithSpaces(TT::C_PAREN);

    // fn_ret ::= spaces MINUS GT spaces type_id
    eatWithSpaces(TT::MINUS);
    eat(TT::GT);
    auto retTypeId = type_id();

    // o_brace fn_content c_brace
    eatWithSpaces(TT::O_BRACE);

    eatEmptyLines();

    eatWithSpaces(TT::C_BRACE);

    auto result = construct<AST::FnDef>(id, std::move(retTypeId), std::move(fnArgs));

    return result;
}

// fn_arg ::= id colon type_id
AST::Node::Ptr Parser::fn_arg()
{
    eatAll(TT::SPACE);

    if(currToken_.type_ == TT::C_PAREN)
        return nullptr;
    if(currToken_.type_ == TT::COMMA)
        eat(TT::COMMA);

    std::string id = eatValueWithSpaces(TT::ID);
    eatWithSpaces(TT::COLON);
    return construct<AST::Variable>(id, type_id());
}

// type_id ::= id (o_brack (int|id) c_brack)?
AST::Node::Ptr Parser::type_id()
{
    auto result = construct<AST::TypeId>(eatValueWithSpaces(TT::ID, EatSpaces::Both));

    if(currToken_.type_ == TT::O_BRACK)
    {
        result->isArray_ = true;

        eatWithSpaces(TT::O_BRACK, EatSpaces::Right);
        if(currToken_.type_ == TT::NUM)
        {
            result->arraySize_ = eatValueWithSpaces(TT::NUM, EatSpaces::Both);
        }
        else
        {
            result->arraySize_ = eatValueWithSpaces(TT::ID, EatSpaces::Both);
        }
        eat(TT::C_BRACK);
    }

    return result;
}

// /// cmd ::= print | call | set | sub
// AST::Node::Ptr Parser::cmd()
// {
//     AST::Node::Ptr result;

//   if(result = tryParse(&Parser::print); result)
//   {
//   }
//   else if(result = tryParse(&Parser::call); result)
//   {
//   }
//   else if(result = tryParse(&Parser::set); result)
//   {
//   }
//   else if(result = tryParse(&Parser::sub); result)
//   {
//   }

//   if(!result)
//   {
//       throw unexpectedToken("cmd");
//   }

//   return result;
// }

void Parser::eat(TokenType tt)
{
    if(tt == currToken_.type_)
    {
        currToken_ = tokenizer_.getNext();
    }
    else
    {
        throw unexpectedToken(tt, "eat");
    }
}

std::string Parser::eatVal(TokenType tt)
{
    if(tt == currToken_.type_)
    {
        std::string result = currToken_.value_;
        currToken_         = tokenizer_.getNext();
        return result;
    }
    else
    {
        throw unexpectedToken(tt, "eat");
    }
}

std::string Parser::eatValueWithSpaces(TokenType tt, EatSpaces policy)
{
    switch(policy)
    {
        case EatSpaces::Left: {
            eatAll(TT::SPACE);
            return eatVal(tt);
        }
        break;

        case EatSpaces::Right: {
            auto result = eatVal(tt);
            eatAll(TT::SPACE);
            return result;
        }
        break;

        case EatSpaces::Both: {
            eatAll(TT::SPACE);
            auto result = eatVal(tt);
            eatAll(TT::SPACE);
            return result;
        }
        break;

        default: break;
    }
}

void Parser::eatWithSpaces(TokenType tt, EatSpaces policy)
{
    switch(policy)
    {
        case EatSpaces::Left: {
            eatAll(TT::SPACE);
            eat(tt);
        }
        break;

        case EatSpaces::Right: {
            eat(tt);
            eatAll(TT::SPACE);
        }
        break;

        case EatSpaces::Both: {
            eatAll(TT::SPACE);
            eat(tt);
            eatAll(TT::SPACE);
        }
        break;

        default: break;
    }
}

void Parser::eatAll(TokenType tt)
{
    while(currToken_.type_ == tt)
        eat(tt);
}

void Parser::eatEmptyLines()
{
    eatAll(TT::SPACE);

    while(currToken_.type_ == TT::EOL)
    {
        eat(TT::EOL);
        eatAll(TT::SPACE);
    }
}

void Parser::checkTokenType(TokenType expected, ValidationSource source)
{
    if(currToken_.type_ != expected)
        throw unexpectedToken(expected, source);
}

void Parser::checkTokenValue(std::string expected, ValidationSource source)
{
    if(currToken_.value_ != expected)
        throw unexpectedValue(expected, source);
}

std::runtime_error Parser::unexpectedValue(std::string expected, ValidationSource source)
{
    std::ostringstream ss;
    ss << "Unexpected token value in line " << tokenizer_.currentLine() << " while parsing '" << source << "'"
       << " [ExpectedValue = '" << expected << "', CurrentToken = " << currToken_ << "]";

    return std::runtime_error(ss.str());
}

std::runtime_error Parser::unexpectedToken(ValidationSource source)
{
    std::ostringstream ss;
    ss << "Unexpected token in line " << tokenizer_.currentLine() << " while parsing '" << source << "'"
       << " [CurrentToken = " << currToken_ << "]";

    return std::runtime_error(ss.str());
}

std::runtime_error Parser::unexpectedToken(TokenType expected, ValidationSource source)
{
    std::ostringstream ss;
    ss << "Unexpected token in line " << tokenizer_.currentLine() << " while parsing '" << source << "'"
       << " [Expected = " << expected << ", "
       << "Actual = " << currToken_.type_ << "]";

    return std::runtime_error(ss.str());
}

}
