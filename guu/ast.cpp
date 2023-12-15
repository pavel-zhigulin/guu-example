#include "ast.h"
#include <iostream>
#include <cassert>

namespace Guu::AST
{

void Printer::visit(Root& root)
{
    indent();
    os() << "(Root)" << std::endl;

    addIndent();
    for(size_t i = 0; i < root.children_.size(); ++i)
    {
        visit(*root.children_[i].get());
    }

    subIndent();
}

void Printer::visit(FnDef& proc)
{
    indent();
    os() << "(FnDef id = '" << proc.id_ << "', retTypeId = '";
    visit(*proc.retTypeId_);
    os() << "')" << std::endl;

    addIndent();
    for(auto& arg: proc.params_)
    {
        visit(*arg);
    }
    subIndent();
}

void Printer::visit(Variable& proc)
{
    indent();
    os() << "(Variable id = '" << proc.id_ << "', typeId = '";
    visit(*proc.typeId_);
    os() << "')" << std::endl;
}

void Printer::visit(TypeId& typeId)
{
    os() << typeId.tname_;
    if(typeId.isArray_)
    {
        os() << "[" << typeId.arraySize_ << "]";
    }
}

void Printer::visit(UnaryOp&)
{
    indent();
    os() << "(UnaryOp)" << std::endl;
}

void Printer::visit(BinOp&)
{
    indent();
    os() << "(BinOp)" << std::endl;
}

void Printer::indent()
{
    os() << std::string(indent_, ' ');
}

}
