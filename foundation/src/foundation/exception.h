#ifndef GAMEFRIENDS_EXCEPTION_H
#define GAMEFRIENDS_EXCEPTION_H

#include "prerequest.h"
#include <exception>
#include <string>
#include <utility>
#include <functional>
#include <cassert>

#define GF_SCOPE_EXIT_NAME(id) GF_CAT(gf_scope_exit_, id)

#define GF_SCOPE_EXIT \
    const GF_NAMESPACE::detail::ScopeExit GF_SCOPE_EXIT_NAME(GF_ID) = \
        GF_NAMESPACE::detail::ScopeExit::relay = [&]

#define check(e) (assert(e && #e))
#define static_check(e) (static_assert(e, #e)) /** BUGS: STATIC_CHECK_BUG */

GF_NAMESPACE_BEGIN

namespace detail
{
    struct ScopeExit
    {
        static std::function<void()> relay;
        std::function<void()> fun_;
        ScopeExit(std::function<void()> fun);
        ~ScopeExit();
    };
}

template <class E, class T, class... Args>
T enforce(T&& value, Args&&... args)
{
    if (!value)
    {
        throw E(std::forward<Args>(args)...);
    }
    return std::forward<T>(value);
}

class Exception : public std::exception
{
private:
    std::string msg_;

public:
    explicit Exception(const std::string& msg);
    std::string msg() const;
    const char* what() const;
};

class FileException : public Exception
{
public:
    explicit FileException(const std::string& msg);
};

GF_NAMESPACE_END

#endif