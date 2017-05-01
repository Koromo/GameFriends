#include "vector2.h"
#include "vector3.h"
#include "math.h"
#include "exception.h"
#include <cmath>

GF_NAMESPACE_BEGIN

const Vector2 Vector2::ZERO = { 0, 0 };
const Vector2 Vector2::UNIT_X = { 1, 0 };
const Vector2 Vector2::UNIT_Y = { 0, 1 };

Vector2::Vector2(float nx, float ny)
    : x(nx)
    , y(ny)
{
}

const float& Vector2::operator [](size_t i) const
{
    return const_cast<Vector2&>(*this)[i];
}

float& Vector2::operator [](size_t i)
{
    switch (i)
    {
    case 0: return x;
    case 1: return y;
    default:
        check(false);
        return x;
    }
}

Vector3 Vector2::xyz(float z) const
{
    return{ x, y, z };
}

bool operator ==(const Vector2& a, const Vector2& b)
{
    return equalf(a.x, b.x) && equalf(a.y, b.y);
}

bool operator !=(const Vector2& a, const Vector2& b)
{
    return !(a == b);
}

const Vector2 operator +(const Vector2& v)
{
    return v;
}

const Vector2 operator -(const Vector2& v)
{
    return{ -v.x, -v.y };
}

const Vector2 operator +(const Vector2& a, const Vector2& b)
{
    return{ a.x + b.x, a.y + b.y };
}

const Vector2 operator -(const Vector2& a, const Vector2& b)
{
    return{ a.x - b.x, a.y - b.y };
}

const Vector2 operator *(const Vector2& v, float k)
{
    return{ v.x * k, v.y * k };
}

const Vector2 operator *(float k, const Vector2& v)
{
    return v * k;
}

const Vector2 operator /(const Vector2& v, float k)
{
    check(!equalf(k, 0));
    const auto invK = 1 / k;
    return v * invK;
}

Vector2& operator +=(Vector2& a, const Vector2& b)
{
    a = a + b;
    return a;
}

Vector2& operator -=(Vector2& a, const Vector2& b)
{
    a = a - b;
    return a;
}

Vector2& operator *=(Vector2& v, float k)
{
    v = v * k;
    return v;
}

Vector2& operator /=(Vector2& v, float k)
{
    check(!equalf(k, 0));
    v = v / k;
    return v;
}

Vector2 scale(const Vector2& v, const Vector2& k)
{
    return{ v.x * k.x, v.y * k.y };
}

Vector2 invScale(const Vector2& v, const Vector2& k)
{
    return{ v.x / k.x, v.y / k.y };
}

float norm(const Vector2& v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

Vector2 normalize(const Vector2& v)
{
    check(!equalf(norm(v), 0));
    return v / norm(v);
}

float dotProduct(const Vector2& a, const Vector2& b)
{
    return a.x * b.x + a.y * b.y;
}

GF_NAMESPACE_END