#ifndef GAMEFRIENDS_RIGIDBODY_H
#define GAMEFRIENDS_RIGIDBODY_H

#include "bulletinc.h"
#include "foundation/prerequest.h"
#include <LinearMath/btMotionState.h>
#include <memory>

GF_NAMESPACE_BEGIN

class Vector3;
class Quaternion;
class CollisionShape;
class RigidBody;

struct PhysicsListener
{
    virtual ~PhysicsListener() = default;
    virtual void move(const Vector3& pos, const Quaternion& q) {}
    virtual void collision(RigidBody& collider) {}
};

class RigidBody
{
private:
    struct MyMotionState : public btDefaultMotionState
    {
        MyMotionState(const btTransform& startTrans = btTransform::getIdentity(),
            const btTransform& centerOfMassOffset = btTransform::getIdentity());
        void setWorldTransform(const btTransform& trans) override;
    };

    std::unique_ptr<btRigidBody> body_;
    MyMotionState motionState_;
    std::shared_ptr<CollisionShape> shape_;
    PhysicsListener* listener_;
    void* usrPtr_;

public:
    RigidBody(const std::shared_ptr<CollisionShape> shape, float mass);

    bool inWorld() const;

    void setPhysicsListener(PhysicsListener* listener);

    void setPosition(const Vector3& pos);
    void setOrientation(const Quaternion& q);
    void setLinearVelocity(const Vector3& vel);
    void setAngularVelocity(const Vector3& vel);

    void applyCentralForce(const Vector3& dir);

    void activate(bool b);

    void setUserPointer(void* p);
    void* userPointer();

    void notifyCollision(RigidBody& collider);
    btRigidBody& btBody();
};

GF_NAMESPACE_END

#endif