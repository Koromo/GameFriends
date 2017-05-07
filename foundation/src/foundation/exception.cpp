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

GF_NAMESPACE_END