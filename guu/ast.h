#pragma once

#include "token.h"

#include <vector>
#include <string>
#include <iosfwd>
#include <variant>
#include <vector>
#include <memory>
#include <optional>

#include "../util/visitor.h"

namespace Guu::AST
{

#define GUU_NODE_TYPE_VALUES(_)        \
    _(Root, "Root node")               \
    _(BinOp, "Binary operations")      \
    _(UnaryOp, "Unary operations")     \
    _(FnDef, "Function definition")    \
    _(Variable, "Variable definition") \
    _(TypeId, "Type Declaration")

// clang-format off
enum class NodeType
{
    #define MAKE_ENUM(x, _) x,
    GUU_NODE_TYPE_VALUES(MAKE_ENUM)
    #undef MAKE_ENUM
};

// clang-format on

struct Node
{
    using Ptr = std::unique_ptr<Node>;

    NodeType type_;

    Node(NodeType nt) : type_(nt)
    {
    }

    Node(const Node&)            = delete;
    Node& operator=(const Node&) = delete;
    Node(Node&&)                 = delete;
    Node& operator=(Node&&)      = delete;

    virtual ~Node() = default;
};

using NodeVec = std::vector<Node::Ptr>;

struct Root : Node
{
    Root() : Node(NodeType::Root)
    {
    }

    NodeVec children_;
};

struct BinOp : Node
{
    BinOp() : Node(NodeType::BinOp)
    {
    }

    Node::Ptr op1_;
    Node::Ptr op2_;
};

struct UnaryOp : Node
{
    UnaryOp() : Node(NodeType::UnaryOp)
    {
    }

    Node::Ptr op_;
};

struct TypeId : Node
{
    TypeId(std::string tname) : Node(NodeType::TypeId), tname_(std::move(tname)), isArray_(false), arraySize_("0")
    {
    }

    std::string tname_;
    bool isArray_;
    std::string arraySize_;
};

struct FnDef : Node

{
    FnDef(std::string id, Node::Ptr retTypeId, NodeVec params)
        : Node(NodeType::FnDef), id_(std::move(id)), retTypeId_(std::move(retTypeId)), params_(std::move(params))
    {
    }

    std::string id_;
    Node::Ptr retTypeId_;
    NodeVec params_;
    NodeVec statements_;
};

struct Variable : Node
{
    Variable(std::string id, Node::Ptr typeId)
        : Node(NodeType::Variable), id_(std::move(id)), typeId_(std::move(typeId))
    {
    }

    std::string id_;
    Node::Ptr typeId_;
    std::optional<std::string> value;
};

namespace detail
{
// clang-format off
#define ADD_TO_TEMPLATE(nodeType, _) , nodeType
using BaseVisitor = util::IVisitorOf<
    util::visitor_policy::generate_default
    GUU_NODE_TYPE_VALUES(ADD_TO_TEMPLATE)
>;
#undef ADD_TO_TEMPLATE
// clang-format on
}

struct Visitor : public detail::BaseVisitor
{
    using detail::BaseVisitor::visit;

    void visit(Node& n)
    {
        // clang-format off
        switch(n.type_)
        {
            #define CALL_VISIT(x,_) case NodeType::x: visit(static_cast<x&>(n)); break;
            GUU_NODE_TYPE_VALUES(CALL_VISIT)
            #undef CALL_VISIT
        }
        // clang-format on
    }

    void visit(Root& r) override
    {
        for(const auto& c: r.children_)
        {
            visit(*c);
        }
    }

    virtual ~Visitor() = default;
};

class Printer : public Visitor
{
    static constexpr int INDENT_STEP = 2;

public:
    using Visitor::visit;

    Printer(std::ostream& os) : os_(os)
    {
    }

    ~Printer() override = default;

    void print(Node& node)
    {
        indent_ = 0;
        visit(node);
    }

    std::ostream& os()
    {
        return os_;
    }

private:
    // clang-format off
    #define OVERRIDE_VISIT(node, _) void visit(node& program) override;
    GUU_NODE_TYPE_VALUES(OVERRIDE_VISIT)
    #undef OVERRIDE_VISIT
    // clang-format on

private:
    void indent();

    void addIndent()
    {
        indent_ += INDENT_STEP;
    }

    void subIndent()
    {
        indent_ -= INDENT_STEP;
    }

private:
    std::ostream& os_;
    int indent_ = 0;
};

}
