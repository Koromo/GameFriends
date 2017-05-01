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
};

Plane normalize(const Plane& p);
float dotCoord(const Plane& p, const Vector3& v);
float dotNormal(const Plane& p, const Vector3& v);
float intersects(const Plane& p, const Vector3& rayOrigin, const Vector3& rayDir); /// TODO: IDK for non unit planes
bool intersects(const Plane& p0, const Plane& p1, const Plane& p2, Vector3* at);  /// TODO: IDK for non unit planes

GF_NAMESPACE_END

#endif