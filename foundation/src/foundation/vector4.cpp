#include "vector4.h"
#include "vector3.h"
#include "matrix44.h"
#include "math.h"
#include "exception.h"
#include <cmath>

GF_NAMESPACE_BEGIN

const Vector4 Vector4::ZERO = { 0, 0, 0, 0 };
const Vector4 Vector4::UNIT_X = { 1, 0, 0, 0 };
const Vector4 Vector4::UNIT_Y = { 0, 1, 0, 0 };
const Vector4 Vector4::UNIT_Z = { 0, 0, 1, 0 };
const Vector4 Vector4::UNIT_W = { 0, 0, 0, 1 };

Vector4::Vector4(float nx, float ny, float nz, float nw)
    : x(nx)
    , y(ny)
    , z(nz)
    , w(nw)
{
}

const float& Vector4::operator [](size_t i) const
{
    return const_cast<Vector4&>(*this)[i];
}

float& Vector4::operator [](size_t i)
{
    switch (i)
    {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    case 3: return w;
    default:
        check(false);
        return x;
    }
}

float Vector4::norm() const
{
    return std::sqrt(x * x + y * y + z * z + w * w);
}

float Vector4::dot(const Vector4& v) const
{
    return x * v.x + y * v.y + z * v.z + w * v.w;
}

Vector3 Vector4::xyz() const
{
    return{ x, y, z };
}

void Vector4::normalize()
{
    *this /= norm();
}

void Vector4::scale(const Vector4& k)
{
    *this = { x * k.x, y * k.y, z * k.z, w * k.w };
}

bool operator ==(const Vector4& a, const Vector4& b)
{
    return equalf(a.x, b.x) && equalf(a.y, b.y) && equalf(a.z, b.z) && equalf(a.w, b.w);
}

bool operator !=(const Vector4& a, const Vector4& b)
{
    return !(a == b);
}

const Vector4 operator +(const Vector4& v)
{
    return v;
}

const Vector4 operator -(const Vector4& v)
{
    return{ -v.x, -v.y, -v.z, -v.w };
}

const Vector4 operator +(const Vector4& a, const Vector4& b)
{
    return{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

const Vector4 operator -(const Vector4& a, const Vector4& b)
{
    return{ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

const Vector4 operator *(const Vector4& v, const Matrix44& m)
{
    return{
        v.x * m(0, 0) + v.y * m(1, 0) + v.z * m(2, 0) + v.w * m(3, 0),
        v.x * m(0, 1) + v.y * m(1, 1) + v.z * m(2, 1) + v.w * m(3, 1),
        v.x * m(0, 2) + v.y * m(1, 2) + v.z * m(2, 2) + v.w * m(3, 2),
        v.x * m(0, 3) + v.y * m(1, 3) + v.z * m(2, 3) + v.w * m(3, 3)
    };
}

const Vector4 operator *(const Vector4& v, float k)
{
    return{ v.x * k, v.y * k, v.z * k, v.w * k };
}

const Vector4 operator *(float k, const Vector4& v)
{
    return v * k;
}

const Vector4 operator /(const Vector4& v, float k)
{
    check(!equalf(k, 0));
    const auto invK = 1 / k;
    return v * invK;
}

Vector4& operator +=(Vector4& a, const Vector4& b)
{
    a = a + b;
    return a;
}

Vector4& operator -=(Vector4& a, const Vector4& b)
{
    a = a - b;
    return a;
}

Vector4& operator *=(Vector4& v, const Matrix44& m)
{
    v = v * m;
    return v;
}

Vector4& operator *=(Vector4& v, float k)
{
    v = v * k;
    return v;
}

Vector4& operator /=(Vector4& v, float k)
{
    v = v / k;
    return v;
}

GF_NAMESPACE_END