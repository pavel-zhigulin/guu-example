#pragma once

#include "lexer.h"
#include "ast.h"

#include <iosfwd>
#include <stdexcept>

/// Guu:
/// ============
/// Where to check:
///     https://mdkrajnak.github.io/ebnftest/
/// Grammar:
///     program    ::= statement (NEWLINE+ statement)*
///     statement  ::= SPACE* cmd SPACE*
///     cmd        ::= print | call | set | sub
///     print      ::= PRINT SPACE param
///     call       ::= CALL SPACE ID
///     set        ::= SET SPACE ID SPACE param
///     sub        ::= SUB SPACE ID
///     param      ::= ID | integer
///     integer    ::= (PLUS | MINUS)? NUM
///     SUB        ::= "sub"
///     PRINT      ::= "print"
///     SET        ::= "set"
///     CALL       ::= "call"
///     NUM        ::= #"[0-9]+"
///     ID         ::= #"[a-zA-Z][a-zA-z0-9]+"
///     NEWLINE    ::= '\n'
///     SPACE      ::= ' '
///     PLUS       ::= '+'
///     MINUS      ::= '-'

namespace Guu
{
class Parser
{
    using TT = TokenType;

public:
    Parser(Tokenizer t)
        : tokenizer_(std::move(t))
        , currToken_(tokenizer_.getNext())
    {
    }

    std::unique_ptr<AST::Node> buildAST() { return program();  }

private:
    std::unique_ptr<AST::Node> program();
    std::unique_ptr<AST::Node> statement();
    std::unique_ptr<AST::Node> cmd();
    std::unique_ptr<AST::Node> print();
    std::unique_ptr<AST::Node> call();
    std::unique_ptr<AST::Node> set();
    std::unique_ptr<AST::Node> sub();
    std::unique_ptr<AST::Node> param();
    std::unique_ptr<AST::Node> integer();

private:
    template<typename Node, typename ...Args>
    std::unique_ptr<Node> construct(Args... args)
    {
        return std::make_unique<Node>(currToken_, tokenizer_.currentLine(), args...);
    }

    std::unique_ptr<AST::Node> tryParse(std::unique_ptr<AST::Node>(Parser::*memFn)());

    void eat(TokenType tt);
    void eatAll(TokenType tt);

    std::runtime_error unexpectedToken(const char* source);
    std::runtime_error unexpectedToken(TokenType expected, const char* source);

private:
    Tokenizer tokenizer_;
    Token currToken_;
};
}