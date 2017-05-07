#ifndef GAMEFRIENDS_EXCEPTION_H
#define GAMEFRIENDS_EXCEPTION_H

#include "prerequest.h"
#include <exception>
#include <string>
#include <utility>
#include <functional>
#include <cassert>

#define check(e) (assert(e && #e))
#define static_check(e) (static_assert(e, #e)) /** BUGS: STATIC_CHECK_BUG */

#define GF_SCOPE_EXIT_NAME(id) GF_CAT(gf_scope_exit_, id)

#define GF_SCOPE_EXIT \
    const GF_NAMESPACE::detail::ScopeExit GF_SCOPE_EXIT_NAME(GF_ID) = \
        GF_NAMESPACE::detail::ScopeExit::relay = [&]

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

class Exception : public std::exception
{
private:
    std::string msg_;

public:
    explicit Exception(const std::string& msg)
        : msg_(msg) {}

    std::string msg() const { return msg_; }
    const char* what() const { return msg_.c_str(); }
};

class Error : public std::exception
{
private:
    std::string msg_;

public:
    explicit Error(const std::string& msg)
        : msg_(msg) {}

    std::string msg() const { return msg_; }
    const char* what() const { return msg_.c_str(); }
};

class EnforceError : public Error
{
public:
    explicit EnforceError(const std::string& msg)
        : Error(msg) {}
};

class FileException : public Exception
{
public:
    explicit FileException(const std::string& msg)
        : Exception(msg) {}
};

template <class E = EnforceError, class T, class... Args>
T enforce(T&& value, Args&&... args) noexcept(false)
{
    if (!value)
    {
        throw E(std::forward<Args>(args)...);
    }
    return std::forward<T>(value);
}

GF_NAMESPACE_END

#endif