#include "collisionshape.h"
#include "foundation/vector3.h"
#include <LinearMath/btVector3.h>

GF_NAMESPACE_BEGIN

bool CollisionShape::dynamic() const
{
    return dynamic_;
}

btCollisionShape& CollisionShape::btShape()
{
    return *shape_;
}

std::shared_ptr<CollisionShape> CollisionShape::StaticPlane(const Vector3& normal)
{
    std::shared_ptr<CollisionShape> ret(new CollisionShape());
    ret->dynamic_ = false;
    ret->shape_.reset(new btStaticPlaneShape(to<btVector3>(normal), 0));
    return ret;
}

std::shared_ptr<CollisionShape> CollisionShape::Box(const Vector3& xyz)
{
    std::shared_ptr<CollisionShape> ret(new CollisionShape());
    ret->shape_.reset(new btBoxShape(to<btVector3>(xyz * 0.5)));
    return ret;
}

GF_NAMESPACE_END