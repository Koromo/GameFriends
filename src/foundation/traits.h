#ifndef GAMEFRIENDS_TRAITS_H
#define GAMEFRIENDS_TRAITS_H

#include "prerequest.h"
#include <type_traits>
#include <utility>

GF_NAMESPACE_BEGIN

namespace detail
{
    template <class T, class U>
    struct isReturnable : std::is_convertible<T, U> {};

    template <class T>
    struct isReturnable<T, void> : std::true_type {};

    template <class... Args>
    struct Args_t
    {
        template <class Fun, class R, class = void>
        struct Callable_t : std::false_type {};

        template <class Fun, class R>
        struct Callable_t<Fun, R,
            std::enable_if_t<
                isReturnable<
                    decltype(std::declval<Fun>()
                        (std::declval<Args>()...)),
                    R
                >::value
            >
        > : std::true_type {};
    };

    template <class Arg, class... Args>
    struct Args_t<Arg, Args...>
    {
        template <class Fun, class R, class = void>
        struct Callable_t : std::false_type {};

        template <class Fun, class R>
        struct Callable_t<Fun, R,
            std::enable_if_t<
                isReturnable<
                    decltype(std::declval<Fun>()
                        (std::declval<Arg>(), std::declval<Args>()...)),
                    R
                >::value
            >
        > : std::true_type {};

        template <class C, class R, class... Sig>
        struct Callable_t<R(C::*)(Sig...), R,
            std::enable_if_t<
                isReturnable<
                    decltype((std::declval<std::remove_pointer_t<Arg>>().*(std::declval<R(C::*)(Sig...)>()))
                        (std::declval<Args>()...)),
                    R
                >::value
            >
        > : std::true_type {};

        template <class C, class R, class... Sig>
        struct Callable_t<R(C::*)(Sig...) const, R,
            std::enable_if_t<
                isReturnable<
                    decltype((std::declval<std::remove_pointer_t<Arg>>().*(std::declval<R(C::*)(Sig...) const>()))
                        (std::declval<Args>()...)),
                    R
                >::value
            >
        > : std::true_type {};
    };
}

template <class Call, class R = void>
struct isCallable;

template <class Fun, class R, class... Args>
struct isCallable<Fun(Args...), R> : detail::Args_t<Args...>::template Callable_t<Fun, R> {};

GF_NAMESPACE_END

#endif