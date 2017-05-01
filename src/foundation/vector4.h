#ifndef GAMEFRIENDS_VECTOR4_H
#define GAMEFRIENDS_VECTOR4_H

#include "prerequest.h"

GF_NAMESPACE_BEGIN

class Vector3;
class Matrix44;

class Vector4
{
public:
    float x, y, z, w;

    Vector4() = default;
    Vector4(float nx, float ny, float nz, float nw);
    Vector4(const Vector4&) = default;

    Vector4& operator =(const Vector4&) = default;

    const float& operator [](size_t i) const;
    float& operator [](size_t i);

    Vector3 xyz() const;

    static const Vector4 ZERO;
    static const Vector4 UNIT_X;
    static const Vector4 UNIT_Y;
    static const Vector4 UNIT_Z;
    static const Vector4 UNIT_W;
};

bool operator ==(const Vector4& a, const Vector4& b);
bool operator !=(const Vector4& a, const Vector4& b);

const Vector4 operator +(const Vector4& v);
const Vector4 operator -(const Vector4& v);

const Vector4 operator +(const Vector4& a, const Vector4& b);
const Vector4 operator -(const Vector4& a, const Vector4& b);
const Vector4 operator *(const Vector4& v, const Matrix44& m);
const Vector4 operator *(const Vector4& v, float k);
const Vector4 operator *(float k, const Vector4& v);
const Vector4 operator /(const Vector4& v, float k);

Vector4& operator +=(Vector4& a, const Vector4& b);
Vector4& operator -=(Vector4& a, const Vector4& b);
Vector4& operator *=(Vector4& v, const Matrix44& m);
Vector4& operator *=(Vector4& v, float k);
Vector4& operator /=(Vector4& v, float k);

Vector4 scale(const Vector4& v, const Vector4& k);
//Vector4 invScale(const Vector4& v, const Vector4& k);
float norm(const Vector4& v);
Vector4 normalize(const Vector4& v);
float dotProduct(const Vector4& a, const Vector4& b);

GF_NAMESPACE_END

#endif
