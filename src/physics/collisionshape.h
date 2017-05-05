#ifndef GAMEFRIENDS_COLLISIONSHAPE_H
#define GAMEFRIENDS_COLLISIONSHAPE_H

#include "bulletinc.h"
#include "foundation/prerequest.h"
#include <memory>

GF_NAMESPACE_BEGIN

class Vector3;

class CollisionShape
{
private:
    std::unique_ptr<btCollisionShape> shape_;
    bool dynamic_ = true;

    CollisionShape() = default;

public:
    bool dynamic() const;
    btCollisionShape& btShape();

    static std::shared_ptr<CollisionShape> StaticPlane(const Vector3& normal);
    static std::shared_ptr<CollisionShape> Box(const Vector3& xyz);
};

GF_NAMESPACE_END

#endif