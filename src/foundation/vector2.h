#ifndef GAMEFRIENDS_VECTOR2_H
#define GAMEFRIENDS_VECTOR2_H

#include "prerequest.h"

GF_NAMESPACE_BEGIN

class Vector3;

class Vector2
{
public:
    float x, y;

    Vector2() = default;
    Vector2(float nx, float ny);
    Vector2(const Vector2&) = default;

    Vector2& operator =(const Vector2&) = default;

    const float& operator [](size_t i) const;
    float& operator [](size_t i);

    float norm() const;
    float dot(const Vector2& v) const;
    Vector3 xyz(float z) const;

    void normalize();
    void scale(const Vector2& k);

    static const Vector2 ZERO;
    static const Vector2 UNIT_X;
    static const Vector2 UNIT_Y;
};

bool operator ==(const Vector2& a, const Vector2& b);
bool operator !=(const Vector2& a, const Vector2& b);

const Vector2 operator +(const Vector2& v);
const Vector2 operator -(const Vector2& v);

const Vector2 operator +(const Vector2& a, const Vector2& b);
const Vector2 operator -(const Vector2& a, const Vector2& b);
const Vector2 operator *(const Vector2& v, float k);
const Vector2 operator *(float k, const Vector2& v);
const Vector2 operator /(const Vector2& v, float k);

Vector2& operator +=(Vector2& a, const Vector2& b);
Vector2& operator -=(Vector2& a, const Vector2& b);
Vector2& operator *=(Vector2& v, float k);
Vector2& operator /=(Vector2& v, float k);

GF_NAMESPACE_END

#endif
