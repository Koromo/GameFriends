#ifndef GAMEFRIENDS_PHYSICSDEBUG_H
#define GAMEFRIENDS_PHYSICSDEBUG_H

#include "../physics/physicsworld.h"

GF_NAMESPACE_BEGIN

class PhysicsDebug : public PhysicsDebugDraw
{
    void line(const Vector3& from, const Vector3& to, const Color& color);
};

GF_NAMESPACE_END

#endif