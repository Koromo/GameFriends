#ifndef GAMEFRIENDS_VECTOR3_H
#define GAMEFRIENDS_VECTOR3_H

#include "prerequest.h"

GF_NAMESPACE_BEGIN

class Vector2;
class Vector4;
class Quaternion;

class Vector3
{
public:
    float x, y, z;

    Vector3() = default;
    Vector3(float nx, float ny, float nz);
    Vector3(const Vector3&) = default;

    Vector3& operator =(const Vector3&) = default;

    const float& operator [](size_t i) const;
    float& operator [](size_t i);

    float norm() const;
    float dot(const Vector3& v) const;
    Vector3 cross(const Vector3& v) const;
    Vector2 xy() const;
    Vector4 xyzw(float w) const;

    void normalize();
    void scale(const Vector3& k);

    static const Vector3 ZERO;
    static const Vector3 UNIT_X;
    static const Vector3 UNIT_Y;
    static const Vector3 UNIT_Z;
};

bool operator ==(const Vector3& a, const Vector3& b);
bool operator !=(const Vector3& a, const Vector3& b);

const Vector3 operator +(const Vector3& v);
const Vector3 operator -(const Vector3& v);

const Vector3 operator +(const Vector3& a, const Vector3& b);
const Vector3 operator -(const Vector3& a, const Vector3& b);
const Vector3 operator *(const Vector3& v, float k);
const Vector3 operator *(float k, const Vector3& v);
const Vector3 operator *(const Quaternion& q, const Vector3& v);
const Vector3 operator /(const Vector3& v, float k);

Vector3& operator +=(Vector3& a, const Vector3& b);
Vector3& operator -=(Vector3& a, const Vector3& b);
Vector3& operator *=(Vector3& v, float k);
Vector3& operator /=(Vector3& v, float k);

GF_NAMESPACE_END

#endif