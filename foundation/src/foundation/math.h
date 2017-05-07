#ifndef GAMEFRIENDS_MATH_H
#define GAMEFRIENDS_MATH_H

#include "exception.h"
#include "prerequest.h"
#include <string>

GF_NAMESPACE_BEGIN

extern const float PI;		/// 3.141592...
extern const float PI_2;	/// PI/2
extern const float PI_4;	/// PI/4
extern const float EPSILON; /// 0.0001f

float radian(float deg) noexcept;
float degree(float rad) noexcept;
bool equalf(float a, float b) noexcept;

template <class T>
T ceiling(T n, T base) noexcept
{
    return static_cast<T>(static_cast<unsigned long long>((n + base - 1) / base)) * base;
}

unsigned crc32(const void* p, size_t length) noexcept;

template <class It>
unsigned hashCombine(It b, It e)
{
    unsigned hc = 0;
    for (It i = b; i != e; ++i)
    {
        hc = 31 * hc + *i;
    }
    return hc;
}

GF_NAMESPACE_END

#endif