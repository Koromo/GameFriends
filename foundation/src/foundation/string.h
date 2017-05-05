#ifndef GAMEFRIENDS_STRING_H
#define GAMEFRIENDS_STRING_H

#include "prerequest.h"
#include <string>
#include <cstdarg>

#ifdef GF_UNICODE
#define GF_TEXT(s) (L ## s)
#elif
#define GF_TEXT(s) (s)
#endif

#define GF_T(s) GF_TEXT(s)

GF_NAMESPACE_BEGIN

std::string narrow(const std::string& s);
std::string narrow(const std::wstring& s);
std::wstring widen(const std::string& s);
std::wstring widen(const std::wstring& s);

std::string tolowers(const std::string& s);
std::wstring tolowers(const std::wstring& s);
std::string touppers(const std::string& s);
std::wstring touppers(const std::wstring& s);

int lowerscmp(const std::string& a, const std::string& b);
int lowerscmp(const std::wstring& a, const std::wstring& b);

size_t strlen(const std::string& s);
size_t strlen(const std::wstring& s);

int vsprintf(char* buffer, const char* fmt, va_list args);
int vsprintf(wchar_t* buffer, const wchar_t* fmt, va_list args);

#ifdef GF_UNICODE
#define charset(str) (widen(str))
using char_t = wchar_t;
#elif
#define charset(str) (narrow(str))
using char_t = char;
#endif

using string_t = std::basic_string<char_t, std::char_traits<char_t>, std::allocator<char_t>>;

GF_NAMESPACE_END

#endif