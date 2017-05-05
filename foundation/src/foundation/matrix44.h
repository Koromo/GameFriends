#ifndef GAMEFRIENDS_MATRIX44_H
#define GAMEFRIENDS_MATRIX44_H

#include "quaternion.h"
#include "prerequest.h"
#include <initializer_list>
#include <utility>

GF_NAMESPACE_BEGIN

class Vector3;
class Vector4;

class Matrix44
{
private:
    float m_[4][4];

public:
    Matrix44() = default;

    Matrix44(
        float n00, float n01, float n02, float n03,
        float n10, float n11, float n12, float n13,
        float n20, float n21, float n22, float n23,
        float n30, float n31, float n32, float n33
        );

    void swap(Matrix44& that);

    Matrix44(const Matrix44& that) = default;
    Matrix44(Matrix44&& that) = default;

    Matrix44(std::initializer_list<float> il);

    Matrix44& operator =(std::initializer_list<float> il);
    Matrix44& operator =(const Matrix44&) = default;
    Matrix44& operator =(Matrix44&&) = default;

    const float& operator ()(size_t r, size_t c) const;
    float& operator ()(size_t r, size_t c);

    Vector4 row(size_t r) const;
    Vector4 column(size_t c) const;
    float determinant() const;
    Matrix44 transpose() const;
    Matrix44 inverse() const;

    static const Matrix44 IDENTITY;
};

bool operator ==(const Matrix44& a, const Matrix44& b);
bool operator !=(const Matrix44& a, const Matrix44& b);

const Matrix44 operator +(const Matrix44& m);
const Matrix44 operator -(const Matrix44& m);

const Matrix44 operator +(const Matrix44& a, const Matrix44& b);
const Matrix44 operator -(const Matrix44& a, const Matrix44& b);
const Matrix44 operator *(const Matrix44& a, const Matrix44& b);
const Matrix44 operator *(const Matrix44& m, float k);
const Matrix44 operator *(float k, const Matrix44& m);
const Matrix44 operator /(const Matrix44& m, float k);

Matrix44& operator +=(Matrix44& a, const Matrix44& b);
Matrix44& operator -=(Matrix44& a, const Matrix44& b);
Matrix44& operator *=(Matrix44& a, const Matrix44& b);
Matrix44& operator *=(Matrix44& m, float k);
Matrix44& operator /=(Matrix44& m, float k);

Matrix44 makeTransformMatrix(const Vector3& scale, const Quaternion& rot, const Vector3& trans);
Matrix44 makeLookAtMatrix(const Vector3& eye, const Vector3& at, const Vector3& up);
Matrix44 makePerspectiveMatrix(float fovY, float aspect, float zn, float zf); /// NOTE: aspect = X/Y
Matrix44 makeOrthoMatrix(float width, float height, float zn, float zf);
Matrix44 makeOrthoMatrix(float left, float right, float bottom, float top, float zn, float zf);

template <>
inline Matrix44 to(const Quaternion& q_)
{
    auto q = q_;
    q.normalize();

    const auto w2 = q.w * 2;
    const auto x2 = q.x * 2;
    const auto y2 = q.y * 2;
    const auto z2 = q.z * 2;

    auto m = Matrix44::IDENTITY;

    m(0, 0) = 1 - y2 * q.y - z2 * q.z;
    m(0, 1) = x2 * q.y + w2 * q.z;
    m(0, 2) = x2 * q.z - w2 * q.y;

    m(1, 0) = x2 * q.y - w2 * q.z;
    m(1, 1) = 1 - x2 * q.x - z2 * q.z;
    m(1, 2) = y2 * q.z + w2 * q.x;

    m(2, 0) = x2 * q.z + w2 * q.y;
    m(2, 1) = y2 * q.z - w2 * q.x;
    m(2, 2) = 1 - x2 * q.x - y2 * q.y;

    return m;
}

GF_NAMESPACE_END

namespace std
{
    template <>
    inline void swap(GF_NAMESPACE::Matrix44& a, GF_NAMESPACE::Matrix44& b)
    {
        a.swap(b);
    }
}

#endif