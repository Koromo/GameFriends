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

    static const AxisAlignedBox NEGATIVE;
};

Vector3 corner(const AxisAlignedBox& box, size_t n);
Vector3 centroid(const AxisAlignedBox& box);
AxisAlignedBox merge(const AxisAlignedBox& box, const Vector3& p);
AxisAlignedBox merge(const AxisAlignedBox& boxA, const AxisAlignedBox& boxB);
bool contains(const AxisAlignedBox& box, const Vector3& p);
bool contains(const AxisAlignedBox& box, const AxisAlignedBox& innerTest);
float intersects(const AxisAlignedBox& box, const Vector3& rayOrigin, const Vector3& rayDir);
AxisAlignedBox transform(const AxisAlignedBox& box, const Matrix44& m);

GF_NAMESPACE_END

#endif