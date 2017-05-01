#ifndef GAMEFRIENDS_WINDOWSINC_H
#define GAMEFRIENDS_WINDOWSINC_H

#include "../foundation/exception.h"
#include "../foundation/prerequest.h"
#include <Windows.h>
#include <windowsx.h>
#include <string>
#include <memory>

#undef max
#undef min
//#undef NEAR
//#undef FAR
//#undef near
//#undef far

GF_NAMESPACE_BEGIN

class WindowsException : public Exception
{
public:
    explicit WindowsException(const std::string& msg)
        : Exception(msg) {}
};

template <class E>
void verify(HRESULT hr, const std::string& msg)
{
    enforce<E>(SUCCEEDED(hr), "Code: " + std::to_string(hr) + "\n" + msg);
}

template <class T>
using ComPtr = std::shared_ptr<T>;

template <class T>
using ComWeakPtr = std::weak_ptr<T>;

template <class T>
ComPtr<T> makeComPtr(T* p)
{
    struct D
    {
        void operator()(T* a)
        {
            a->Release();
        }
    };
    return ComPtr<T>(p, D());
}

GF_NAMESPACE_END

#endif