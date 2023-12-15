// #pragma once

// #include "ast.h"

// #include <unordered_map>
// #include <stack>
// #include <vector>
// #include <string>

// namespace Guu
// {
// class Interpreter : public AST::Visitor
// {
//     using TT          = TokenType;
//     using Address     = std::string;
//     using Instruction = const AST::Node*;
//     using AST::Visitor::visit;

// public:
//     explicit Interpreter(std::unique_ptr<AST::Node> program);

//   void interpret();

//   void stepInto();
//   void stepOver();
//   void bt() const;
//   void printVars() const;

// private:
//     void compile();

//   void visit(const AST::FnDef& proc) override;
//   void visit(const AST::BinOp& binOp) override;
//   void visit(const AST::UnaryOp& op) override;

// private:
//     void execute(Instruction i);
//     void execute(const AST::BinOp& op);
//     void execute(const AST::UnaryOp& op);

// private:
//     bool isEnd() const;

//   void defineProc(const Address& address);
//   bool hasProc(const Address& address);
//   std::vector<Instruction>& findProc(const Address& address);
//   void addInstruction(Instruction i);

//   void setVariable(const Address& address, const std::string& value);
//   std::string getVariable(const Address& address);

//   void moveNext();
//   void jumpTo(const Address& address);

// private:
//     std::unordered_map<Address, std::vector<Instruction>> text_;
//     using InstIter = decltype(decltype(text_)::value_type::second)::const_iterator;
//     InstIter currentInstruction_{};

//   std::unique_ptr<AST::Node> prg_;
//   std::string currProc_;
//   std::unordered_map<Address, std::string> globalMemory_{};
//   std::stack< std::pair<std::string, InstIter> > stack_{};
// };
// }
