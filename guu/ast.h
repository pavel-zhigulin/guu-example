#pragma once

#include "token.h"

#include <vector>
#include <string>
#include <iosfwd>
#include <variant>
#include <vector>
#include <memory>

#include "../util/visitor.h"

namespace Guu::AST
{

#define NODE_TYPES(_) \
    _(Root) \
    _(BinOp) \
    _(UnaryOp) \
    _(Param) \
    _(ProcDecl) \

#define COMMA_RIGHT(x) x,
#define COMMA_LEFT(x) ,x

enum class NodeType
{
    NODE_TYPES(COMMA_RIGHT)
};

struct Node
{
    explicit Node(Token t, NodeType nt) : type_(nt), token_(std::move(t)) {}

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    Node(Node&&) = delete;
    Node& operator=(Node&&) = delete;

    NodeType type_;
    Token token_;

    virtual ~Node() = default;
};

#define NODE_CONSTRUCTOR(x) x(Token t) : Node(std::move(t), NodeType::x)

struct Root : Node
{
    NODE_CONSTRUCTOR(Root) {}
    std::vector<std::unique_ptr<Node>> children_;
};

struct BinOp : Node
{
    NODE_CONSTRUCTOR(BinOp) {}
    std::unique_ptr<Node> op1_;
    std::unique_ptr<Node> op2_;
};

struct UnaryOp : Node
{
    NODE_CONSTRUCTOR(UnaryOp) {}
    std::unique_ptr<Node> op_;
};

struct Param : Node
{
    NODE_CONSTRUCTOR(Param) {}
    Param(Token t, std::string val) : Param(t)
    {
        value_ = std::move(val);
    }

    std::string value_ = token_.value_;
};

struct ProcDecl : Node
{
    NODE_CONSTRUCTOR(ProcDecl) {}
    std::string name_;
};

#undef NODE_CONSTRUCTOR

namespace detail
{
    using BaseVisitor =
        util::IVisitorOf<
            util::visitor_policy::generate_default
            NODE_TYPES(COMMA_LEFT)
        >;
}

struct Visitor
    : public detail::BaseVisitor
{
    using detail::BaseVisitor::visit;

    void visit(const Node& n)
    {
#ifdef __MSC_VER
#pragma warning( push )
#pragma warning( error: 4062)
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wswitch"
#endif

#define CASE(x) case NodeType::x: visit(static_cast<const x&>(n)); break;
        switch (n.type_)
        {
            NODE_TYPES(CASE)
        }
#undef CASE

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef __MSC_VER
#pragma warning( pop )
#endif
    }

    void visit(const Root& r)
    {
        for(const auto& c : r.children_)
        {
            visit(*c);
        }
    }

    virtual ~Visitor() = default;
};

class Printer
    : public Visitor
{
    static constexpr int INDENT_STEP = 2;

public:
    using Visitor::visit;

    Printer(std::ostream& os)
        : os_(os)
    {}

    ~Printer() override = default;

    void print(const Node& node)
    {
        indent_ = 0;
        visit(node);
    }

    std::ostream& os() { return os_;  }

private:
    void visit(const Root& program) override;
    void visit(const ProcDecl& sub) override;
    void visit(const UnaryOp& op) override;
    void visit(const BinOp& binop) override;
    void visit(const Param& param) override;

private:
    void indent();
    void addIndent() { indent_ += INDENT_STEP; }
    void subIndent() { indent_ -= INDENT_STEP; }

private:
    std::ostream& os_;
    int indent_ = 0;
};

#undef NODE_TYPES
#undef COMMA_LEFT
#undef COMMA_RIGHT

}
