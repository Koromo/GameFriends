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

Vector3 AxisAlignedBox::corner(size_t n) const
{
    return{ (n & 1) ? minimum.x : maximum.x,
        (n & 2) ? minimum.y : maximum.y,
        (n & 4) ? minimum.z : maximum.z };
}

Vector3 AxisAlignedBox::centroid() const
{
    return{ (minimum + maximum) * 0.5 };
}

AxisAlignedBox AxisAlignedBox::transform(const Matrix44& m) const
{
    auto result = AxisAlignedBox::NEGATIVE;
    for (int i = 0; i < 8; ++i)
    {
        const auto v = corner(i).xyzw(1) * m;
        result.merge(v.xyz() / v.w);
    }
    return result;
}

void AxisAlignedBox::merge(const Vector3& p)
{
    minimum.x = std::min(minimum.x, p.x);
    minimum.y = std::min(minimum.y, p.y);
    minimum.z = std::min(minimum.z, p.z);
    maximum.x = std::max(maximum.x, p.x);
    maximum.y = std::max(maximum.y, p.y);
    maximum.z = std::max(maximum.z, p.z);
}

void AxisAlignedBox::merge(const AxisAlignedBox& box)
{
    merge(box.minimum);
    merge(box.maximum);
}

bool AxisAlignedBox::contains(const Vector3& p) const
{
    return minimum.x <= p.x && p.x <= maximum.x &&
        minimum.y <= p.y && p.y <= maximum.y &&
        minimum.z <= p.z && p.z <= maximum.z;
}

bool AxisAlignedBox::contains(const AxisAlignedBox& box) const
{
    return contains(box.minimum) && contains(box.maximum);
}

float AxisAlignedBox::intersects(const Vector3& origin, const Vector3& dir) const
{
    const std::array<Plane, 6> sides = {
        Plane(Vector3::UNIT_X, -minimum.x),
        Plane(-Vector3::UNIT_X, maximum.x),
        Plane(Vector3::UNIT_Y, -minimum.y),
        Plane(-Vector3::UNIT_Y, maximum.y),
        Plane(Vector3::UNIT_Z, -minimum.z),
        Plane(-Vector3::UNIT_Z, maximum.z)
    };

    for (int i = 0; i < 6; ++i)
    {
        const auto d = sides[i].intersects(origin, dir);
        if (d + EPSILON < 0)
        {
            continue;
        }

        const auto hitAt = origin + dir * d;
        bool inside = true;
        for (int j = 0; j < 6 && inside; ++j)
        {
            if (j == i)
            {
                continue;
            }
            inside = sides[j].dotCoord(hitAt) + EPSILON >= 0;
        }

        if (inside)
        {
            return d;
        }
    }

    return -INFINITY;
}

GF_NAMESPACE_END