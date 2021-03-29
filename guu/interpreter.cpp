#include "interpreter.h"

#include <cassert>
#include <set>
#include <iostream>

namespace Guu
{

Interpreter::Interpreter(std::unique_ptr<AST::Node> program)
    : prg_(std::move(program))
{
    compile();
}

void Interpreter::interpret()
{
    jumpTo("main");
    while (!isEnd())
    {
        execute(*currentInstruction_);
    }
}

void Interpreter::execute(Interpreter::Instruction i)
{
    switch (i->type_)
    {
        case AST::NodeType::BinOp:
            return execute(static_cast<const AST::BinOp&>(*i));

        case AST::NodeType::UnaryOp:
            return execute(static_cast<const AST::UnaryOp&>(*i));

        default:
            throw std::runtime_error("Unknown instruction executed");
    }
}

void Interpreter::execute(const AST::BinOp& op)
{
    switch (op.token_.type_)
    {
        case TT::SET:
        {
            auto&& id = dynamic_cast<AST::Param&>(*op.op1_);
            assert(id.token_.type_ == TT::ID);

            auto&& val = dynamic_cast<AST::Param&>(*op.op2_);
            assert((std::set{TT::NUM, TT::ID}.count(val.token_.type_)));

            setVariable(
                id.value_,
                val.token_.type_ == TokenType::NUM ? val.value_ : getVariable(val.value_)
            );

            moveNext();
        } break;

        default:
            throw std::runtime_error("Unknown instuction executed");
    }
}

void Interpreter::execute(const AST::UnaryOp& op)
{
    auto&& par = dynamic_cast<AST::Param&>(*op.op_);

    switch (op.token_.type_)
    {
        case TT::PRINT:
        {
            assert((std::set{TT::NUM, TT::ID}.count(par.token_.type_)));
            auto&& val = par.token_.type_ == TT::NUM ? par.value_ : getVariable(par.value_);
            std::cout << val << std::endl;
            moveNext();
        } break;

        case TT::CALL:
        {
            assert(par.token_.type_ == TT::ID);
            jumpTo(par.value_);
        } break;

        default:
            throw std::runtime_error("Unknown instuction executed");
    }
}

void Interpreter::compile()
{
    visit(*prg_);

    currProc_ = "";
    currentInstruction_ = {};
}

void Interpreter::visit(const AST::ProcDecl& proc)
{
    if (hasProc(proc.name_))
    {
        throw std::runtime_error("Double definition of '" + proc.name_ + "'");
    }

    defineProc(proc.name_);
    currProc_ = proc.name_;
}

void Interpreter::visit(const AST::BinOp& binOp)
{
    switch (binOp.token_.type_)
    {
        case TT::SET:
        {
            addInstruction( &binOp );
        } break;

        default:
            throw std::runtime_error("Unknown BinOp");
    }
}

void Interpreter::visit(const AST::UnaryOp& op)
{
    switch (op.token_.type_)
    {
        case TT::PRINT:
        case TT::CALL:
        {
            addInstruction( &op );
        } break;

        default:
            throw std::runtime_error("Unknown UnaryOp");
    }
}

void Interpreter::defineProc(const Interpreter::Address& address)
{
    text_[address] = {};
}

bool Interpreter::hasProc(const Interpreter::Address& address)
{
    return text_.find(address) != text_.end();
}

void Interpreter::addInstruction(Interpreter::Instruction i)
{
    findProc(currProc_).emplace_back( std::move(i) );
}

std::vector<Interpreter::Instruction>& Interpreter::findProc(const Interpreter::Address& address)
{
    try
    {
        return text_.at(address);
    }
    catch(const std::out_of_range&)
    {
        throw std::runtime_error("Unknown sub '" + address + "'");
    }
}

void Interpreter::setVariable(const Interpreter::Address& address, const std::string& value)
{
    globalMemory_[address] = value;
}

std::string Interpreter::getVariable(const Interpreter::Address& address)
{
    try
    {
        return globalMemory_.at(address);
    }
    catch(const std::out_of_range&)
    {
        throw std::runtime_error("Unknown variable '" + address + "'");
    }
}

void Interpreter::moveNext()
{
    currentInstruction_++;
    while (findProc(currProc_).cend() == currentInstruction_)
    {
        std::tie(currProc_, currentInstruction_) = stack_.top();
        stack_.pop();
    }
}

void Interpreter::jumpTo(const Interpreter::Address& address)
{
    if (stack_.size())
    {
        stack_.push( {currProc_, currentInstruction_ + 1} );
        currentInstruction_ = findProc(address).cbegin();
        currProc_ = address;

        if (stack_.size() > 1024)
        {
            throw std::runtime_error("!!! STACK OVERFLOW !!!");
        }
    }
    else
    {
        if (address != "main")
        {
            throw std::runtime_error("PLEASE START FROM MAIN");
        }
        currentInstruction_ = findProc("main").cbegin();
        currProc_ = address;
        stack_.push( {currProc_, currentInstruction_ } );
    }
}

bool Interpreter::isEnd() const
{
    return stack_.size() == 0;
}

}
