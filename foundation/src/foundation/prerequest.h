#ifndef GAMEFRIENDS_PREREQUEST_H
#define GAMEFRIENDS_PREREQUEST_H

#include <cstdint> /// TODO: Environment dependence

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#ifdef _UNICODE
    #define GF_UNICODE
#endif

#ifdef _DEBUG
    #define GF_DEBUG
#endif

#ifdef _WIN32
    #define GF_WINDOWS

    #ifdef _WIN64
        #define GF_x64
    #else
        #define GF_x86
    #endif
#endif

#ifdef __COUNTER__
    #define GF_COUNTER __COUNTER__
    #define GF_ID __COUNTER__
    #define GF_UNIQUE_ID __COUNTER__
#else
    #define GF_ID __LINE__
#endif

#define GF_CAT(a, b) (a ## b)

#define GF_API
#define GF_NOAPI

#define GF_NAMESPACE gamefriends
#define GF_NAMESPACE_BEGIN namespace GF_NAMESPACE {
#define GF_NAMESPACE_END }

namespace GF_NAMESPACE
{
    template <class T, class U> T to(const U& u);

    using int8 = int8_t;
    using int16 = int16_t;
    using int32 = int32_t;
    using int64 = int64_t;

    using uint8 = uint8_t;
    using uint16 = uint16_t;
    using uint32 = uint32_t;
    using uint64 = uint64_t;
}

namespace friends = GF_NAMESPACE;

#endif