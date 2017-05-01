#include "physicsdebug.h"
#include "../scene/debugdraw.h"

GF_NAMESPACE_BEGIN

void PhysicsDebug::line(const Vector3& from, const Vector3& to, const Color& color)
{
    debugDraw.line(from, to, color);
}

GF_NAMESPACE_END