#include "matrix44.h"
#include "vector3.h"
#include "vector4.h"
#include "math.h"
#include "exception.h"
#include <algorithm>
#include <memory>
#include <cmath>

GF_NAMESPACE_BEGIN

const Matrix44 Matrix44::IDENTITY = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

Matrix44::Matrix44(
    float n00, float n01, float n02, float n03,
    float n10, float n11, float n12, float n13,
    float n20, float n21, float n22, float n23,
    float n30, float n31, float n32, float n33
    )
    : m_()
{
    m_[0][0] = n00;
    m_[0][1] = n01;
    m_[0][2] = n02;
    m_[0][3] = n03;

    m_[1][0] = n10;
    m_[1][1] = n11;
    m_[1][2] = n12;
    m_[1][3] = n13;

    m_[2][0] = n20;
    m_[2][1] = n21;
    m_[2][2] = n22;
    m_[2][3] = n23;

    m_[3][0] = n30;
    m_[3][1] = n31;
    m_[3][2] = n32;
    m_[3][3] = n33;
}

Matrix44::Matrix44(std::initializer_list<float> il)
    : m_()
{
    *this = il;
}

void Matrix44::swap(Matrix44& that)
{
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            std::swap(m_[r][c], that.m_[r][c]);
        }
    }
}

Matrix44& Matrix44::operator =(std::initializer_list<float> il)
{
    check(il.size() == 16);

    auto it = std::begin(il);
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            m_[r][c] = *it;
            ++it;
        }
    }
    return *this;
}

const float& Matrix44::operator ()(size_t r, size_t c) const
{
    return const_cast<Matrix44&>(*this)(r, c);
}

float& Matrix44::operator ()(size_t r, size_t c)
{
    check(r < 4 && c < 4);
    return m_[r][c];
}

Vector4 Matrix44::row(size_t r) const
{
    return{ m_[r][0], m_[r][1], m_[r][2], m_[r][3] };
}

Vector4 Matrix44::column(size_t c) const
{
    return{ m_[0][c], m_[1][c], m_[2][c], m_[3][c] };
}

float Matrix44::determinant() const
{
    return (m_[0][0] * m_[1][1] - m_[0][1] * m_[1][0]) * (m_[2][2] * m_[3][3] - m_[2][3] * m_[3][2]) -
        (m_[0][0] * m_[1][2] - m_[0][2] * m_[1][0]) * (m_[2][1] * m_[3][3] - m_[2][3] * m_[3][1]) +
        (m_[0][0] * m_[1][3] - m_[0][3] * m_[1][0]) * (m_[2][1] * m_[3][2] - m_[2][2] * m_[3][1]) +
        (m_[0][1] * m_[1][2] - m_[0][2] * m_[1][1]) * (m_[2][0] * m_[3][3] - m_[2][3] * m_[3][0]) -
        (m_[0][1] * m_[1][3] - m_[0][3] * m_[1][1]) * (m_[2][0] * m_[3][2] - m_[2][2] * m_[3][0]) +
        (m_[0][2] * m_[1][3] - m_[0][3] * m_[1][2]) * (m_[2][0] * m_[3][1] - m_[2][1] * m_[3][0]);
}

Matrix44 Matrix44::transpose() const
{
    return{
        m_[0][0], m_[1][0], m_[2][0], m_[3][0],
        m_[0][1], m_[1][1], m_[2][1], m_[3][1],
        m_[0][2], m_[1][2], m_[2][2], m_[3][2],
        m_[0][3], m_[1][3], m_[2][3], m_[3][3]
    };
}

Matrix44 Matrix44::inverse() const
{
    const auto det = determinant();
    check(!equalf(det, 0));
    const auto invDet = 1 / det;
    return{
        invDet * (m_[1][1] * (m_[2][2] * m_[3][3] - m_[2][3] * m_[3][2]) + m_[1][2] * (m_[2][3] * m_[3][1] - m_[2][1] * m_[3][3]) + m_[1][3] * (m_[2][1] * m_[3][2] - m_[2][2] * m_[3][1])),
        invDet * (m_[2][1] * (m_[0][2] * m_[3][3] - m_[0][3] * m_[3][2]) + m_[2][2] * (m_[0][3] * m_[3][1] - m_[0][1] * m_[3][3]) + m_[2][3] * (m_[0][1] * m_[3][2] - m_[0][2] * m_[3][1])),
        invDet * (m_[3][1] * (m_[0][2] * m_[1][3] - m_[0][3] * m_[1][2]) + m_[3][2] * (m_[0][3] * m_[1][1] - m_[0][1] * m_[1][3]) + m_[3][3] * (m_[0][1] * m_[1][2] - m_[0][2] * m_[1][1])),
        invDet * (m_[0][1] * (m_[1][3] * m_[2][2] - m_[1][2] * m_[2][3]) + m_[0][2] * (m_[1][1] * m_[2][3] - m_[1][3] * m_[2][1]) + m_[0][3] * (m_[1][2] * m_[2][1] - m_[1][1] * m_[2][2])),

        invDet * (m_[1][2] * (m_[2][0] * m_[3][3] - m_[2][3] * m_[3][0]) + m_[1][3] * (m_[2][2] * m_[3][0] - m_[2][0] * m_[3][2]) + m_[1][0] * (m_[2][3] * m_[3][2] - m_[2][2] * m_[3][3])),
        invDet * (m_[2][2] * (m_[0][0] * m_[3][3] - m_[0][3] * m_[3][0]) + m_[2][3] * (m_[0][2] * m_[3][0] - m_[0][0] * m_[3][2]) + m_[2][0] * (m_[0][3] * m_[3][2] - m_[0][2] * m_[3][3])),
        invDet * (m_[3][2] * (m_[0][0] * m_[1][3] - m_[0][3] * m_[1][0]) + m_[3][3] * (m_[0][2] * m_[1][0] - m_[0][0] * m_[1][2]) + m_[3][0] * (m_[0][3] * m_[1][2] - m_[0][2] * m_[1][3])),
        invDet * (m_[0][2] * (m_[1][3] * m_[2][0] - m_[1][0] * m_[2][3]) + m_[0][3] * (m_[1][0] * m_[2][2] - m_[1][2] * m_[2][0]) + m_[0][0] * (m_[1][2] * m_[2][3] - m_[1][3] * m_[2][2])),

        invDet * (m_[1][3] * (m_[2][0] * m_[3][1] - m_[2][1] * m_[3][0]) + m_[1][0] * (m_[2][1] * m_[3][3] - m_[2][3] * m_[3][1]) + m_[1][1] * (m_[2][3] * m_[3][0] - m_[2][0] * m_[3][3])),
        invDet * (m_[2][3] * (m_[0][0] * m_[3][1] - m_[0][1] * m_[3][0]) + m_[2][0] * (m_[0][1] * m_[3][3] - m_[0][3] * m_[3][1]) + m_[2][1] * (m_[0][3] * m_[3][0] - m_[0][0] * m_[3][3])),
        invDet * (m_[3][3] * (m_[0][0] * m_[1][1] - m_[0][1] * m_[1][0]) + m_[3][0] * (m_[0][1] * m_[1][3] - m_[0][3] * m_[1][1]) + m_[3][1] * (m_[0][3] * m_[1][0] - m_[0][0] * m_[1][3])),
        invDet * (m_[0][3] * (m_[1][1] * m_[2][0] - m_[1][0] * m_[2][1]) + m_[0][0] * (m_[1][3] * m_[2][1] - m_[1][1] * m_[2][3]) + m_[0][1] * (m_[1][0] * m_[2][3] - m_[1][3] * m_[2][0])),

        invDet * (m_[1][0] * (m_[2][2] * m_[3][1] - m_[2][1] * m_[3][2]) + m_[1][1] * (m_[2][0] * m_[3][2] - m_[2][2] * m_[3][0]) + m_[1][2] * (m_[2][1] * m_[3][0] - m_[2][0] * m_[3][1])),
        invDet * (m_[2][0] * (m_[0][2] * m_[3][1] - m_[0][1] * m_[3][2]) + m_[2][1] * (m_[0][0] * m_[3][2] - m_[0][2] * m_[3][0]) + m_[2][2] * (m_[0][1] * m_[3][0] - m_[0][0] * m_[3][1])),
        invDet * (m_[3][0] * (m_[0][2] * m_[1][1] - m_[0][1] * m_[1][2]) + m_[3][1] * (m_[0][0] * m_[1][2] - m_[0][2] * m_[1][0]) + m_[3][2] * (m_[0][1] * m_[1][0] - m_[0][0] * m_[1][1])),
        invDet * (m_[0][0] * (m_[1][1] * m_[2][2] - m_[1][2] * m_[2][1]) + m_[0][1] * (m_[1][2] * m_[2][0] - m_[1][0] * m_[2][2]) + m_[0][2] * (m_[1][0] * m_[2][1] - m_[1][1] * m_[2][0]))
    };
}

bool operator ==(const Matrix44& a, const Matrix44& b)
{
    return equalf(a(0, 0), b(0, 0)) && equalf(a(0, 1), b(0, 1)) && equalf(a(0, 2), b(0, 2)) && equalf(a(0, 3), b(0, 3))
        && equalf(a(1, 0), b(1, 0)) && equalf(a(1, 1), b(1, 1)) && equalf(a(1, 2), b(1, 2)) && equalf(a(1, 3), b(1, 3))
        && equalf(a(2, 0), b(2, 0)) && equalf(a(2, 1), b(2, 1)) && equalf(a(2, 2), b(2, 2)) && equalf(a(2, 3), b(2, 3))
        && equalf(a(3, 0), b(3, 0)) && equalf(a(3, 1), b(3, 1)) && equalf(a(3, 2), b(3, 2)) && equalf(a(3, 3), b(3, 3));
}

bool operator !=(const Matrix44& a, const Matrix44& b)
{
    return !(a == b);
}

const Matrix44 operator +(const Matrix44& m)
{
    return m;
}

const Matrix44 operator -(const Matrix44& m)
{
    return{
        -m(0, 0), -m(0, 1), -m(0, 2), -m(0, 3),
        -m(1, 0), -m(1, 1), -m(1, 2), -m(1, 3),
        -m(2, 0), -m(2, 1), -m(2, 2), -m(2, 3),
        -m(3, 0), -m(3, 1), -m(3, 2), -m(3, 3)
    };
}

const Matrix44 operator +(const Matrix44& a, const Matrix44& b)
{
    return{
        a(0, 0) + b(0, 0), a(0, 1) + b(0, 1), a(0, 2) + b(0, 2), a(0, 3) + b(0, 3),
        a(1, 0) + b(1, 0), a(1, 1) + b(1, 1), a(1, 2) + b(1, 2), a(1, 3) + b(1, 3),
        a(2, 0) + b(2, 0), a(2, 1) + b(2, 1), a(2, 2) + b(2, 2), a(2, 3) + b(2, 3),
        a(3, 0) + b(3, 0), a(3, 1) + b(3, 1), a(3, 2) + b(3, 2), a(3, 3) + b(3, 3)
    };
}

const Matrix44 operator -(const Matrix44& a, const Matrix44& b)
{
    return{
        a(0, 0) - b(0, 0), a(0, 1) - b(0, 1), a(0, 2) - b(0, 2), a(0, 3) - b(0, 3),
        a(1, 0) - b(1, 0), a(1, 1) - b(1, 1), a(1, 2) - b(1, 2), a(1, 3) - b(1, 3),
        a(2, 0) - b(2, 0), a(2, 1) - b(2, 1), a(2, 2) - b(2, 2), a(2, 3) - b(2, 3),
        a(3, 0) - b(3, 0), a(3, 1) - b(3, 1), a(3, 2) - b(3, 2), a(3, 3) - b(3, 3)
    };
}

const Matrix44 operator *(const Matrix44& a, const Matrix44& b)
{
    return{
        a(0, 0) * b(0, 0) + a(0, 1) * b(1, 0) + a(0, 2) * b(2, 0) + a(0, 3) * b(3, 0),
        a(0, 0) * b(0, 1) + a(0, 1) * b(1, 1) + a(0, 2) * b(2, 1) + a(0, 3) * b(3, 1),
        a(0, 0) * b(0, 2) + a(0, 1) * b(1, 2) + a(0, 2) * b(2, 2) + a(0, 3) * b(3, 2),
        a(0, 0) * b(0, 3) + a(0, 1) * b(1, 3) + a(0, 2) * b(2, 3) + a(0, 3) * b(3, 3),

        a(1, 0) * b(0, 0) + a(1, 1) * b(1, 0) + a(1, 2) * b(2, 0) + a(1, 3) * b(3, 0),
        a(1, 0) * b(0, 1) + a(1, 1) * b(1, 1) + a(1, 2) * b(2, 1) + a(1, 3) * b(3, 1),
        a(1, 0) * b(0, 2) + a(1, 1) * b(1, 2) + a(1, 2) * b(2, 2) + a(1, 3) * b(3, 2),
        a(1, 0) * b(0, 3) + a(1, 1) * b(1, 3) + a(1, 2) * b(2, 3) + a(1, 3) * b(3, 3),

        a(2, 0) * b(0, 0) + a(2, 1) * b(1, 0) + a(2, 2) * b(2, 0) + a(2, 3) * b(3, 0),
        a(2, 0) * b(0, 1) + a(2, 1) * b(1, 1) + a(2, 2) * b(2, 1) + a(2, 3) * b(3, 1),
        a(2, 0) * b(0, 2) + a(2, 1) * b(1, 2) + a(2, 2) * b(2, 2) + a(2, 3) * b(3, 2),
        a(2, 0) * b(0, 3) + a(2, 1) * b(1, 3) + a(2, 2) * b(2, 3) + a(2, 3) * b(3, 3),

        a(3, 0) * b(0, 0) + a(3, 1) * b(1, 0) + a(3, 2) * b(2, 0) + a(3, 3) * b(3, 0),
        a(3, 0) * b(0, 1) + a(3, 1) * b(1, 1) + a(3, 2) * b(2, 1) + a(3, 3) * b(3, 1),
        a(3, 0) * b(0, 2) + a(3, 1) * b(1, 2) + a(3, 2) * b(2, 2) + a(3, 3) * b(3, 2),
        a(3, 0) * b(0, 3) + a(3, 1) * b(1, 3) + a(3, 2) * b(2, 3) + a(3, 3) * b(3, 3)
    };
}

const Matrix44 operator *(const Matrix44& m, float k)
{
    return{
        m(0, 0) * k, m(0, 1) * k, m(0, 2) * k, m(0, 3) * k,
        m(1, 0) * k, m(1, 1) * k, m(1, 2) * k, m(1, 3) * k,
        m(2, 0) * k, m(2, 1) * k, m(2, 2) * k, m(2, 3) * k,
        m(3, 0) * k, m(3, 1) * k, m(3, 2) * k, m(3, 3) * k
    };
}

const Matrix44 operator *(float k, const Matrix44& m)
{
    return m * k;
}

const Matrix44 operator /(const Matrix44& m, float k)
{
    check(!equalf(k, 0));
    const auto invK = 1 / k;
    return m * invK;
}

Matrix44& operator +=(Matrix44& a, const Matrix44& b)
{
    a = a + b;
    return a;
}

Matrix44& operator -=(Matrix44& a, const Matrix44& b)
{
    a = a - b;
    return a;
}

Matrix44& operator *=(Matrix44& a, const Matrix44& b)
{
    a = a * b;
    return a;
}

Matrix44& operator *=(Matrix44& m, float k)
{
    m = m * k;
    return m;
}

Matrix44& operator /=(Matrix44& m, float k)
{
    m = m / k;
    return m;
}

Matrix44 makeTransformMatrix(const Vector3& scale, const Quaternion& rot, const Vector3& trans)
{
    auto m = Matrix44::IDENTITY;

    m(0, 0) = scale.x;
    m(1, 1) = scale.y;
    m(2, 2) = scale.z;

    m *= to<Matrix44>(rot);

    m(3, 0) = trans.x;
    m(3, 1) = trans.y;
    m(3, 2) = trans.z;

    return m;
}

Matrix44 makeLookAtMatrix(const Vector3& eye, const Vector3& at, const Vector3& up)
{
    /// I dont understand yet
    auto zAxis = at - eye;
    zAxis.normalize();

    auto xAxis = up.cross(zAxis);
    xAxis.normalize();

    const auto yAxis = zAxis.cross(xAxis);

    return{
        xAxis.x, yAxis.x, zAxis.x, 0,
        xAxis.y, yAxis.y, zAxis.y, 0,
        xAxis.z, yAxis.z, zAxis.z, 0,
        -(xAxis.dot(eye)), -(yAxis.dot(eye)), -(zAxis.dot(eye)), 1
    };
}

Matrix44 makePerspectiveMatrix(float fovY, float aspect, float zn, float zf)
{
    check(fovY >= 0);
    check(aspect > 0);
    check(zn < zf);

    const auto h = 1 / std::tan(fovY * 0.5f);
    const auto w = h / aspect;
    const auto q = zf / (zf - zn);
    const auto p = -q * zn;
    return{
        w, 0, 0, 0,
        0, h, 0, 0,
        0, 0, q, 1,
        0, 0, p, 0
    };
}

Matrix44 makeOrthoMatrix(float width, float height, float zn, float zf)
{
    check(width > 0);
    check(height > 0);
    check(zn < zf);
    return{
        2 / width, 0, 0, 0,
        0, 2 / height, 0, 0,
        0, 0, 1 / (zf - zn), 0,
        0, 0, -zn / (zf - zn), 1
    };
}

Matrix44 makeOrthoMatrix(float left, float right, float bottom, float top, float zn, float zf)
{
    check(left < right);
    check(bottom < top);
    check(zn < zf);
    return{
        2 / (right - left), 0, 0, 0,
        0, 2 / (top - bottom), 0, 0,
        0, 0, 1 / (zf - zn), 0,
        (left + right) / (left - right), (bottom + top) / (bottom - top), zn / (zn - zf), 1
    };
}

GF_NAMESPACE_END