#include "plane.h"
#include "math.h"
#include <cmath>

GF_NAMESPACE_BEGIN

Plane::Plane(const Vector3& n_, float d_)
    : n(n_)
    , d(d_)
{
}

Plane normalize(const Plane& p)
{
    const auto k = 1 / norm(p.n);
    return{ p.n * k, p.d * k };
}

float dotCoord(const Plane& p, const Vector3& v)
{
    return dotProduct(p.n, v) + p.d;
}

float dotNormal(const Plane& p, const Vector3& v)
{
    return dotProduct(p.n, v);
}

float intersects(const Plane& p, const Vector3& rayOrigin, const Vector3& rayDir)
{
    const auto unitRay = normalize(rayDir);
    const auto cosTheta = dotNormal(p, unitRay);
    if (equalf(cosTheta, 0)) // Parallel ray
    {
        return -INFINITY;
    }
    return -dotCoord(p, rayOrigin) / cosTheta;
}

bool intersects(const Plane& p0, const Plane& p1, const Plane& p2, Vector3* at)
{
    /// TODO: I dont understand yet
    const auto n01 = crossProduct(p0.n, p1.n);
    const auto n12 = crossProduct(p1.n, p2.n);
    const auto n20 = crossProduct(p2.n, p0.n);
    const auto cosTheta = dotProduct(p0.n, n12);
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