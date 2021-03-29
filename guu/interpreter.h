#pragma once

#include "ast.h"

#include "../util/meta_helpers.hpp"

namespace Guu
{
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
        using namespace util::meta_helpers;

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
        using namespace util::meta_helpers;

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
}
