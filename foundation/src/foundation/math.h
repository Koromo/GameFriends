#ifndef GAMEFRIENDS_MATH_H
#define GAMEFRIENDS_MATH_H

#include "prerequest.h"

GF_NAMESPACE_BEGIN

extern const float PI;		/// 3.141592...
extern const float PI_2;	/// PI/2
extern const float PI_4;	/// PI/4
extern const float EPSILON; /// 0.0001

float radian(float deg);
float degree(float rad);
bool equalf(float a, float b);

template <class T>
T ceiling(T n, T base)
{
    return static_cast<T>(static_cast<unsigned long long>((n + base - 1) / base)) * base;
}

unsigned crc32(const void* p, size_t length);

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