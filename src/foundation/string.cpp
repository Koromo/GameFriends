#include "string.h"
#include <vector>
#include <algorithm>
#include <cctype>
#include <cwctype>

GF_NAMESPACE_BEGIN

std::string narrow(const std::string& s)
{
    return s;
}

std::string narrow(const std::wstring& s)
{
    const auto length = s.length();
    std::vector<char> multi(length + 1, '\0');
    std::wcstombs(multi.data(), s.data(), length);
    return multi.data();
}

std::wstring widen(const std::string& s)
{
    const auto length = s.length();
    std::vector<wchar_t> wide(length + 1, L'\0');
    std::mbstowcs(wide.data(), s.data(), length);
    return wide.data();
}

std::wstring widen(const std::wstring& s)
{
    return s;
}

std::string tolowers(const std::string& s)
{
    std::string lowers = s;
    std::transform(std::cbegin(s), std::cend(s), std::begin(lowers), &std::tolower);
    return lowers;
}

std::wstring tolowers(const std::wstring& s)
{
    std::wstring lowers = s;
    std::transform(std::cbegin(s), std::cend(s), std::begin(lowers), &std::towlower);
    return lowers;
}

std::string touppers(const std::string& s)
{
    std::string uppers = s;
    std::transform(std::cbegin(s), std::cend(s), std::begin(uppers), &std::toupper);
    return uppers;
}

std::wstring touppers(const std::wstring& s)
{
    std::wstring uppers = s;
    std::transform(std::cbegin(s), std::cend(s), std::begin(uppers), &std::towupper);
    return uppers;
}

int lowerscmp(const std::string& a, const std::string& b)
{
    return std::strcmp(tolowers(a).c_str(), tolowers(b).c_str());
}

int lowerscmp(const std::wstring& a, const std::wstring& b)
{
    return std::wcscmp(tolowers(a).c_str(), tolowers(b).c_str());

}

size_t strlen(const std::string& s)
{
    return s.length();
}

size_t strlen(const std::wstring& s)
{
    return s.length();
}

int vsprintf(char* buffer, const char* fmt, va_list args)
{
    return std::vsprintf(buffer, fmt, args);
}

int vsprintf(wchar_t* buffer, const wchar_t* fmt, va_list args)
{
    return std::vswprintf(buffer, fmt, args);
}

GF_NAMESPACE_END