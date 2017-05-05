#include "exception.h"

GF_NAMESPACE_BEGIN

std::function<void()> detail::ScopeExit::relay;

detail::ScopeExit::ScopeExit(std::function<void()> fun)
    : fun_(fun)
{
}

detail::ScopeExit::~ScopeExit()
{
    fun_();
}

Exception::Exception(const std::string& msg)
    : msg_(msg)
{
}

std::string Exception::msg() const
{
    return msg_;
}

const char* Exception::what() const
{
    return msg_.c_str();
}

FileException::FileException(const std::string& msg)
    : Exception(msg)
{
}

GF_NAMESPACE_END