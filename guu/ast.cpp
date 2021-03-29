#include "ast.h"
#include <iostream>
#include <cassert>

namespace Guu::AST
{

void Printer::visit(const Root& root)
{
    indent();
    os() << "(Root)" << std::endl;

    addIndent();
    for(size_t i = 0; i < root.children_.size(); ++i)
    {
        visit( *root.children_[i].get() );
    }

    subIndent();
}

void Printer::visit(const ProcDecl& proc)
{
    indent();
    os() << "(ProcDecl '" << proc.name_ << "')" << std::endl;
}

void Printer::visit(const UnaryOp& op)
{
    indent();
    os() << "(UnaryOp '" << op.token_.value_ << "')" << std::endl;
    addIndent();
    assert(op.op_);
    visit(*op.op_);
    subIndent();
}

void Printer::visit(const BinOp& binop)
{
    indent();
    os() << "(BinOp " << binop.token_.value_ << ")" << std::endl;
    addIndent();
    assert(binop.op1_);
    visit(*binop.op1_);

    assert(binop.op2_);
    visit(*binop.op2_);
    subIndent();
}

void Printer::visit(const Param& param)
{
    indent();
    os() << "(Param<" << param.token_.type_ << "> '" << param.value_ << "')" << std::endl;
}

void Printer::indent()
{
    os() << std::string(indent_, ' ');
}

}
