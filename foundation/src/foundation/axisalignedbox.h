#ifndef GAMEFRIENDS_AXISALIGNEDBOX_H
#define GAMEFRIENDS_AXISALIGNEDBOX_H

#include "vector3.h"
#include "prerequest.h"

GF_NAMESPACE_BEGIN

class Matrix44;

struct AxisAlignedBox
{
    static const size_t RTF = 0 | 0 | 0;
    static const size_t LTF = 1 | 0 | 0;
    static const size_t RBF = 0 | 2 | 0;
    static const size_t LBF = 1 | 2 | 0;
    static const size_t RTN = 0 | 0 | 4;
    static const size_t LTN = 1 | 0 | 4;
    static const size_t RBN = 0 | 2 | 4;
    static const size_t LBN = 1 | 2 | 4;

    Vector3 minimum;
    Vector3 maximum;

    AxisAlignedBox() = default;
    AxisAlignedBox(const Vector3& min, const Vector3& max);

    Vector3 corner(size_t n) const;
    Vector3 centroid() const;
    AxisAlignedBox transform(const Matrix44& m) const;

    void merge(const Vector3& p);
    void merge(const AxisAlignedBox& box);

    bool contains(const Vector3& p) const;
    bool contains(const AxisAlignedBox& box) const;
    float intersects(const Vector3& origin, const Vector3& dir) const; /// NODE: unit dir

    static const AxisAlignedBox NEGATIVE;
};

GF_NAMESPACE_END

#endif