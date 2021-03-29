#pragma once

namespace util::meta_helpers
{

/// Just helper for std::visit, took from example:
/// https://en.cppreference.com/w/cpp/utility/variant/visit
template<typename... Ts> struct overloaded: Ts ... { using Ts::operator()...; };
template<typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

/// Holds list of types
template <typename ...Types>
struct type_list
{
    template <
        template <typename...> typename Target
    >
    using apply = Target<Types...>;

    template <
        template <typename> typename Wrapper,
        template <typename...> typename Target
    >
    using apply_wrapped = Target<Wrapper<Types>...>;
};

/// Applies types in TypeList to Target
///
/// Example:
///    \code{.cpp}
///       using valid_types = type_list<int, std::string>;
///       using value_t = apply_to<std::variant, valid_types>;
///
///       static_assert( std::is_same_v<std::variant<int, std::string>, value_t> );
///    \endcode
template <
    template <typename...> typename Target,
    typename TypeList
>
using apply_to = typename TypeList::template apply<Target>;

template <
    template <typename> typename Wrapper,
    template <typename...> typename Target,
    typename TypeList
>
using apply_wrapped_to = typename TypeList::template apply_wrapped<Wrapper, Target>;

}