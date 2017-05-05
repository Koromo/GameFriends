#ifndef GAMEFRIENDS_QUATERNION_H
#define GAMEFRIENDS_QUATERNION_H

#include "prerequest.h"

GF_NAMESPACE_BEGIN

class Vector3;

class Quaternion
{
public:
    float w, x, y, z;

    Quaternion() = default;
    Quaternion(float nw, float nx, float ny, float nz);
    Quaternion(const Quaternion&) = default;

    Quaternion& operator =(const Quaternion&) = default;

    const float& operator [](size_t i) const;
    float& operator [](size_t i);

    Vector3 axis() const;
    float angle() const;
    float norm() const;
    Quaternion inverse() const;
    Quaternion conjugate() const;
    float dot(const Quaternion& q) const;

    void normalize();

    static const Quaternion IDENTITY;
};

bool operator ==(const Quaternion& a, const Quaternion& b);
bool operator !=(const Quaternion& a, const Quaternion& b);

const Quaternion operator +(const Quaternion& q);
const Quaternion operator -(const Quaternion& q);

const Quaternion operator +(const Quaternion& a, const Quaternion& b);
const Quaternion operator -(const Quaternion& a, const Quaternion& b);
const Quaternion operator *(const Quaternion& a, const Quaternion& b);
const Quaternion operator *(const Quaternion& q, float k);

Quaternion& operator +=(Quaternion& a, const Quaternion& b);
Quaternion& operator -=(Quaternion& a, const Quaternion& b);
Quaternion& operator *=(Quaternion& a, const Quaternion& b);
Quaternion& operator *=(Quaternion& q, float k);

Quaternion makeQuaternion(const Vector3& axis, float rad); // NODE: Need unit axis

GF_NAMESPACE_END

#endif