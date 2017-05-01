#include "quaternion.h"
#include "vector3.h"
#include "math.h"
#include "exception.h"
#include <cmath>

GF_NAMESPACE_BEGIN

const Quaternion Quaternion::IDENTITY = { 1, 0, 0, 0 };

Quaternion::Quaternion(float nw, float nx, float ny, float nz)
    : w(nw)
    , x(nx)
    , y(ny)
    , z(nz)
{
}

const float& Quaternion::operator [](size_t i) const
{
    return const_cast<Quaternion&>(*this)[i];
}

float& Quaternion::operator [](size_t i)
{
    switch (i)
    {
    case 0: return w;
    case 1: return x;
    case 2: return y;
    case 3: return z;
    default:
        check(false);
        return w; // For warnings
    }
}

bool operator ==(const Quaternion& a, const Quaternion& b)
{
    return equalf(a.w, b.w) && equalf(a.x, b.x) && equalf(a.y, b.y) && equalf(a.z, b.z);
}

bool operator !=(const Quaternion& a, const Quaternion& b)
{
    return !(a == b);
}

const Quaternion operator +(const Quaternion& q)
{
    return q;
}

const Quaternion operator -(const Quaternion& q)
{
    return{ -q.w, -q.x, -q.y, -q.z };
}

const Quaternion operator +(const Quaternion& a, const Quaternion& b)
{
    return{ a.w + b.w, a.x + b.x, a.y + b.y, a.z + b.z };
}
const Quaternion operator -(const Quaternion& a, const Quaternion& b)
{
    return{ a.w - b.w, a.x - b.x, a.y - b.y, a.z - b.z };
}

const Quaternion operator *(const Quaternion& a, const Quaternion& b)
{
    return{
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z,
        a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x
    };
}

const Quaternion operator *(const Quaternion& q, float k)
{
    return{ q.w * k, q.x * k, q.y * k, q.z * k };
}

Quaternion& operator +=(Quaternion& a, const Quaternion& b)
{
    a = a + b;
    return a;
}

Quaternion& operator -=(Quaternion& a, const Quaternion& b)
{
    a = a - b;
    return a;
}

Quaternion& operator *=(Quaternion& a, const Quaternion& b)
{
    a = a * b;
    return a;
}

Quaternion& operator *=(Quaternion& q, float k)
{
    q = q * k;
    return q;
}

float norm(const Quaternion& q)
{
    return std::sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
}

Quaternion normalize(const Quaternion& q)
{
    check(!equalf(norm(q), 0));
    return q * (1 / norm(q));
}

Quaternion conjugate(const Quaternion& q)
{
    return{ q.w, -q.x, -q.y, -q.z };
}

Quaternion inverse(const Quaternion& q)
{
    check(!equalf(norm(q), 0));
    const auto mag = norm(q);
    const auto conj = conjugate(q);
    return conj * (1 / (mag * mag));
}

float dotProduct(const Quaternion& a, const Quaternion& b)
{
    return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
}

Quaternion makeQuaternion(const Vector3& axis, float rad)
{
    const auto unitAxis = normalize(axis);
    const auto halfAngle = rad * 0.5f;
    const auto c = std::cos(halfAngle);
    const auto s = std::sin(halfAngle);
    return{ c, unitAxis.x * s, unitAxis.y * s, unitAxis.z * s };
}

Vector3 axis(const Quaternion& q)
{
    const auto unitQ = normalize(q);
    const Vector3 v = { unitQ.x, unitQ.y, unitQ.z };

    if (v == Vector3::ZERO) // Rotate axis is not defined
    {
        return Vector3::UNIT_X;
    }

    const auto invSin = 1 / std::sin(angle(unitQ) * 0.5f);
    return v * invSin;
}

float angle(const Quaternion& q)
{
    const auto unitQ = normalize(q);
    return std::acos(unitQ.w) * 2;
}

GF_NAMESPACE_END