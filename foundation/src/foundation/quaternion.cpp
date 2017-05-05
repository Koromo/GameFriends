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

Vector3 Quaternion::axis() const
{
    auto q = *this;
    q.normalize();

    const Vector3 v = { q.x, q.y, q.z };

    if (v == Vector3::ZERO) // Rotate axis is not defined
    {
        return Vector3::UNIT_X;
    }

    const auto invSin = 1 / std::sin(q.angle() * 0.5f);
    return v * invSin;
}

float Quaternion::angle() const
{
    auto q = *this;
    q.normalize();
    return std::acos(q.w) * 2;
}

float Quaternion::norm() const
{
    return std::sqrt(w * w + x * x + y * y + z * z);
}

Quaternion Quaternion::inverse() const
{
    check(!equalf(norm(), 0));
    const auto mag = norm();
    const auto conj = conjugate();
    return conj * (1 / (mag * mag));
}

Quaternion Quaternion::conjugate() const
{
    return{ w, -x, -y, -z };
}

float Quaternion::dot(const Quaternion& q) const
{
    return w * q.w + x * q.x + y * q.y + z * q.z;
}

void Quaternion::normalize()
{
    check(!equalf(norm(), 0));
    *this *= (1 / norm());
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

Quaternion makeQuaternion(const Vector3& axis, float rad)
{
    const auto halfAngle = rad * 0.5f;
    const auto c = std::cos(halfAngle);
    const auto s = std::sin(halfAngle);
    return{ c, axis.x * s, axis.y * s, axis.z * s };
}

GF_NAMESPACE_END