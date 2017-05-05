#ifndef GAMEFRIENDS_UTILITY_H
#define GAMEFRIENDS_UTILITY_H

#include "prerequest.h"
#include <utility>

GF_NAMESPACE_BEGIN

template <size_t... Indices>
struct IndexSequence
{
    static constexpr size_t length = sizeof...(Indices);
};

namespace detail
{
    template <size_t N, size_t... Indices>
    struct IndexSeq : IndexSeq<N - 1, N - 1, Indices...> {};

    template <size_t... Indices>
    struct IndexSeq<0, Indices...>
    {
        using type = IndexSequence<Indices...>;
    };
}

template <size_t N>
struct IndexSequence_t : detail::IndexSeq<N>::type {};

template <class T>
class Counter
{
    T i;

public:
    Counter() : i(0) {}
    explicit Counter(T init) : i(init) {}
    T operator ()() { return i++; }
};

GF_NAMESPACE_END

#endif
