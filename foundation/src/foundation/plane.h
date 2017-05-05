#ifndef GAMEFRIENDS_PLANE_H
#define GAMEFRIENDS_PLANE_H

#include "vector3.h"
#include "prerequest.h"

GF_NAMESPACE_BEGIN

struct Plane
{
    Vector3 n;
    float d;

    Plane() = default;
    Plane(const Vector3& n_, float d_);

    float dotCoord(const Vector3& v) const;
    float dotNormal(const Vector3& v) const;

    /// NODE: unit dir
    /// TODO: IDK when non unit planes
    float intersects(const Vector3& origin, const Vector3& dir) const;

    void normalize();
};

bool intersects(const Plane& p0, const Plane& p1, const Plane& p2, Vector3* at = nullptr); /// TODO: IDK when non unit planes

GF_NAMESPACE_END

#endif