#include "physicsworld.h"
#include "../foundation/color.h"
#include "rigidbody.h"
#include <unordered_map>

#include "collisionshape.h"
#include <algorithm>
#include <cassert>

GF_NAMESPACE_BEGIN

void PhysicsDebugDraw::drawLine(const btVector3& from, const btVector3& to_, const btVector3& color)
{
    const Color c = { color.x(), color.y(), color.z(), 1 };
    line(to<Vector3>(from), to<Vector3>(to_), c);
}

namespace
{
    void tickCallback(btDynamicsWorld* world, btScalar)
    {
        const auto cllidedObjects = static_cast<std::unordered_map<RigidBody*, std::unordered_set<RigidBody*>>*>(world->getWorldUserInfo());
        const auto numManifolds = world->getDispatcher()->getNumManifolds();
        for (int i = 0; i < numManifolds; i++)
        {
            const auto manifold = world->getDispatcher()->getManifoldByIndexInternal(i);
            auto bodyA = static_cast<RigidBody*>(manifold->getBody0()->getUserPointer());
            auto bodyB = static_cast<RigidBody*>(manifold->getBody1()->getUserPointer());
            if (bodyA < bodyB)
            {
                cllidedObjects->operator[](bodyA).emplace(bodyB);
            }
            else
            {
                cllidedObjects->operator[](bodyB).emplace(bodyA);
            }
        }
    }
}

PhysicsWorld::PhysicsWorld()
    : config_()
    , dispather_()
    , broadphase_()
    , solver_()
    , world_()
    , debugDrawer_(nullptr)
{
    config_.reset(new btDefaultCollisionConfiguration());
    dispather_.reset(new btCollisionDispatcher(config_.get()));
    broadphase_.reset(new btDbvtBroadphase());
    solver_.reset(new btSequentialImpulseConstraintSolver());
    world_.reset(new btDiscreteDynamicsWorld(dispather_.get(), broadphase_.get(), solver_.get(), config_.get()));
    world_->setInternalTickCallback(tickCallback);
}

PhysicsWorld::~PhysicsWorld()
{
    clearWorld();
    debugDrawer_ = nullptr;
    world_.reset();
    solver_.reset();
    broadphase_.reset();
    dispather_.reset();
    config_.reset();
}

void PhysicsWorld::clearWorld()
{
    for (int i = world_->getNumCollisionObjects() - 1; i >= 0; --i)
    {
        const auto obj = world_->getCollisionObjectArray()[i];
        world_->removeCollisionObject(obj);
    }
}

void PhysicsWorld::setDebugDrawer(PhysicsDebugDraw* drawer)
{
    debugDrawer_ = drawer;
    world_->setDebugDrawer(drawer);
}

void PhysicsWorld::addRigidBody(RigidBody& body)
{
    world_->addRigidBody(&body.btBody());

}

void PhysicsWorld::removeRigidBody(RigidBody& body)
{
    world_->removeRigidBody(&body.btBody());
}

void PhysicsWorld::stepSimulation(float dt_s)
{
    static const auto FIXED_TIME_STEP = 0.01666666754f;
    std::unordered_map<RigidBody*, std::unordered_set<RigidBody*>> collidedObjects;

    world_->setWorldUserInfo(&collidedObjects);
    world_->stepSimulation(dt_s, static_cast<int>(dt_s / FIXED_TIME_STEP + 1.0001f), FIXED_TIME_STEP);
    world_->setWorldUserInfo(nullptr);

    for (const auto& bodyA : collidedObjects)
    {
        for (const auto& bodyB : bodyA.second)
        {
            bodyA.first->notifyCollision(*bodyB);
            bodyB->notifyCollision(*bodyA.first);
        }
    }

    if (debugDrawer_)
    {
        world_->debugDrawWorld();
    }
}

GF_NAMESPACE_END