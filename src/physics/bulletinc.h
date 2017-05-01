#ifndef GAMEFRIENDS_BULLETINC_H
#define GAMEFRIENDS_BULLETINC_H

#include "../foundation/vector3.h"
#include "../foundation/quaternion.h"
#include "../foundation/prerequest.h"
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

GF_NAMESPACE_BEGIN

template <>
inline btVector3 to(const Vector3& v)
{
    return btVector3(v.x, v.y, v.z);
}

template <>
inline Vector3 to(const btVector3& v)
{
    return{ v.x(), v.y(), v.z() };
}

template <>
inline btQuaternion to(const Quaternion& q)
{
    return btQuaternion(q.x, q.y, q.z, q.w);
}

template <>
inline Quaternion to(const btQuaternion& q)
{
    return{ q.w(), q.x(), q.y(), q.z() };
}

GF_NAMESPACE_END

#endif