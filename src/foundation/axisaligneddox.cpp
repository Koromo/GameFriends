#include "axisalignedbox.h"
#include "plane.h"
#include "math.h"
#include "vector3.h"
#include "vector4.h"
#include <algorithm>
#include <array>
#include <cfloat>

GF_NAMESPACE_BEGIN

const AxisAlignedBox AxisAlignedBox::NEGATIVE = { { FLT_MAX,  FLT_MAX,  FLT_MAX }, { -FLT_MAX,  -FLT_MAX,  -FLT_MAX } };

AxisAlignedBox::AxisAlignedBox(const Vector3& min, const Vector3& max)
    : minimum(min)
    , maximum(max)
{
}

Vector3 corner(const AxisAlignedBox& box, size_t n)
{
    return{ (n & 1) ? box.minimum.x : box.maximum.x,
        (n & 2) ? box.minimum.y : box.maximum.y,
        (n & 4) ? box.minimum.z : box.maximum.z };
}

Vector3 centroid(const AxisAlignedBox& box)
{
    return{ (box.minimum + box.maximum) * 0.5 };
}

AxisAlignedBox merge(const AxisAlignedBox& box, const Vector3& p)
{
    AxisAlignedBox ret;
    ret.minimum.x = std::min(box.minimum.x, p.x);
    ret.minimum.y = std::min(box.minimum.y, p.y);
    ret.minimum.z = std::min(box.minimum.z, p.z);
    ret.maximum.x = std::max(box.maximum.x, p.x);
    ret.maximum.y = std::max(box.maximum.y, p.y);
    ret.maximum.z = std::max(box.maximum.z, p.z);
    return ret;
}

AxisAlignedBox merge(const AxisAlignedBox& boxA, const AxisAlignedBox& boxB)
{
    return merge(merge(boxA, boxB.minimum), boxB.maximum);
}

bool contains(const AxisAlignedBox& box, const Vector3& p)
{
    return box.minimum.x <= p.x && p.x <= box.maximum.x &&
        box.minimum.y <= p.y && p.y <= box.maximum.y &&
        box.minimum.z <= p.z && p.z <= box.maximum.z;
}

bool contains(const AxisAlignedBox& box, const AxisAlignedBox& innerTest)
{
    return contains(box, innerTest.minimum) && contains(box, innerTest.maximum);
}

float intersects(const AxisAlignedBox& box, const Vector3& rayOrigin, const Vector3& rayDir)
{
    const auto unitRay = normalize(rayDir);
    const std::array<Plane, 6> sides = {
        Plane(Vector3::UNIT_X, -box.minimum.x),
        Plane(-Vector3::UNIT_X, box.maximum.x),
        Plane(Vector3::UNIT_Y, -box.minimum.y),
        Plane(-Vector3::UNIT_Y, box.maximum.y),
        Plane(Vector3::UNIT_Z, -box.minimum.z),
        Plane(-Vector3::UNIT_Z, box.maximum.z)
    };

    for (int i = 0; i < 6; ++i)
    {
        const auto d = intersects(sides[i], rayOrigin, unitRay);
        if (d + EPSILON < 0)
        {
            continue;
        }

        const auto hitAt = rayOrigin + unitRay * d;
        bool inside = true;
        for (int j = 0; j < 6 && inside; ++j)
        {
            if (j == i)
            {
                continue;
            }
            inside = dotCoord(sides[j], hitAt) + EPSILON >= 0;
        }

        if (inside)
        {
            return d;
        }
    }

    return -INFINITY;
}

AxisAlignedBox transform(const AxisAlignedBox& box, const Matrix44& m)
{
    auto ret = AxisAlignedBox::NEGATIVE;
    for (int i = 0; i < 8; ++i)
    {
        const auto v = corner(box, i).xyzw(1) * m;
        ret = merge(ret, v.xyz() / v.w);
    }
    return ret;
}

GF_NAMESPACE_END