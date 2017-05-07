#include "plane.h"
#include "exception.h"
#include "math.h"
#include <cmath>

GF_NAMESPACE_BEGIN

Plane::Plane(const Vector3& n_, float d_)
    : n(n_)
    , d(d_)
{
}

float Plane::dotCoord(const Vector3& v) const
{
    return n.dot(v) + d;
}

float Plane::dotNormal(const Vector3& v) const
{
    return n.dot(v);
}

float Plane::intersects(const Vector3& origin, const Vector3& dir) const
{
    const auto cosTheta = dotNormal(dir);
    if (equalf(cosTheta, 0)) // Parallel ray
    {
        return -INFINITY;
    }
    return -dotCoord(origin) / cosTheta;
}

void Plane::normalize()
{
    check(!equalf(n.norm(), 0));
    const auto k = 1 / n.norm();
    *this = { n * k, d * k };
}

bool intersects(const Plane& p0, const Plane& p1, const Plane& p2, Vector3* at)
{
    /// TODO: I dont understand yet
    const auto n01 = p0.n.cross(p1.n);
    const auto n12 = p1.n.cross(p2.n);
    const auto n20 = p2.n.cross(p0.n);
    const auto cosTheta = p0.n.dot(n12);
    if (equalf(cosTheta, 0))
    {
        return false;
    }
    if (at)
    {
        *at = (n01 * -p2.d + n12 * -p0.d + n20 * -p1.d) / cosTheta;
    }
    return true;
}

GF_NAMESPACE_END