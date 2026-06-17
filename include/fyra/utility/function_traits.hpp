#pragma once

#include <type_traits>

namespace fyra::utility
{

template <typename T, typename = void>
struct function_traits;

template <typename Ret, typename... Args>
struct function_traits<Ret(Args...)>
{
    using return_type = Ret;
    using args_tuple = std::tuple<Args...>;

    template <size_t N>
    using arg_at = std::tuple_element_t<N, args_tuple>;
};

template <typename Ret, typename... Args>
struct function_traits<Ret (*)(Args...)>
    : function_traits<Ret(Args...)>
{
};

template <typename Ret, typename Class, typename... Args>
struct function_traits<Ret (Class::*)(Args...)>
    : function_traits<Ret(Args...)>
{
    using class_type = Class;
};

#define DECLARE_FUNCTION_TRAITS_WITH_CV(cv)                   \
    template <typename Ret, typename Class, typename... Args> \
    struct function_traits<Ret (Class::*)(Args...) cv>        \
        : function_traits<Ret (Class::*)(Args...)>            \
    {                                                         \
    };

DECLARE_FUNCTION_TRAITS_WITH_CV(const)
DECLARE_FUNCTION_TRAITS_WITH_CV(volatile)
DECLARE_FUNCTION_TRAITS_WITH_CV(const volatile)

DECLARE_FUNCTION_TRAITS_WITH_CV(noexcept)
DECLARE_FUNCTION_TRAITS_WITH_CV(const noexcept)
DECLARE_FUNCTION_TRAITS_WITH_CV(volatile noexcept)
DECLARE_FUNCTION_TRAITS_WITH_CV(const volatile noexcept)

DECLARE_FUNCTION_TRAITS_WITH_CV(&)
DECLARE_FUNCTION_TRAITS_WITH_CV(const&)
DECLARE_FUNCTION_TRAITS_WITH_CV(volatile&)
DECLARE_FUNCTION_TRAITS_WITH_CV(const volatile&)

DECLARE_FUNCTION_TRAITS_WITH_CV(& noexcept)
DECLARE_FUNCTION_TRAITS_WITH_CV(const& noexcept)
DECLARE_FUNCTION_TRAITS_WITH_CV(volatile& noexcept)
DECLARE_FUNCTION_TRAITS_WITH_CV(const volatile& noexcept)

DECLARE_FUNCTION_TRAITS_WITH_CV(&&)
DECLARE_FUNCTION_TRAITS_WITH_CV(const&&)
DECLARE_FUNCTION_TRAITS_WITH_CV(volatile&&)
DECLARE_FUNCTION_TRAITS_WITH_CV(const volatile&&)

DECLARE_FUNCTION_TRAITS_WITH_CV(&& noexcept)
DECLARE_FUNCTION_TRAITS_WITH_CV(const&& noexcept)
DECLARE_FUNCTION_TRAITS_WITH_CV(volatile&& noexcept)
DECLARE_FUNCTION_TRAITS_WITH_CV(const volatile&& noexcept)

template <typename Lambda>
struct function_traits<Lambda, std::void_t<decltype(&Lambda::operator())>>
    : function_traits<decltype(&Lambda::operator())>
{
};

} // namespace fyra::utility