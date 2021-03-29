#pragma once

#include <variant>

namespace util
{

enum class visitor_policy
{
    none,
    generate_default
};

template <typename T, visitor_policy = visitor_policy::none>
struct IVisitor
{
    virtual void visit(const T&) = 0;
    virtual ~IVisitor() = default;
};

template <typename T>
struct IVisitor<T, visitor_policy::generate_default>
{
    virtual void visit(const T&){}
    virtual ~IVisitor() = default;
};

template<visitor_policy vp, typename ...T>
struct IVisitorOf : public IVisitor<T, vp>...
{
    using Variant = std::variant<T...>;
    using IVisitor<T, vp>::visit...;

    virtual ~IVisitorOf() = default;
};


}
