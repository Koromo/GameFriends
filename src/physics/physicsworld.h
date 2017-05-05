#ifndef GAMEFRIENDS_PHYSICSWORLD_H
#define GAMEFRIENDS_PHYSICSWORLD_H

#include "bulletinc.h"
#include "foundation/prerequest.h"
#include <LinearMath/btIDebugDraw.h>
#include <memory>
#include <unordered_set>

GF_NAMESPACE_BEGIN

class RigidBody;
class Vector3;
struct Color;

class PhysicsDebugDraw : public btIDebugDraw
{
    virtual void line(const Vector3& from, const Vector3& to, const Color& color) = 0;

    void drawLine(const btVector3& from, const btVector3& to_, const btVector3& color);
    void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&) {}
    void reportErrorWarning(const char*) {}
    void draw3dText(const btVector3&, const char*) {}
    void setDebugMode(int) {}
    int getDebugMode() const { return btIDebugDraw::DBG_DrawWireframe; }
};

class PhysicsWorld
{
private:
    std::unique_ptr<btCollisionConfiguration> config_;
    std::unique_ptr<btDispatcher> dispather_;
    std::unique_ptr<btBroadphaseInterface> broadphase_;
    std::unique_ptr<btConstraintSolver> solver_;
    std::unique_ptr<btDynamicsWorld> world_;

    btIDebugDraw* debugDrawer_;

public:
    PhysicsWorld();
    ~PhysicsWorld();

    void clearWorld();

    void setDebugDrawer(PhysicsDebugDraw* drawer);

    void addRigidBody(RigidBody& body);
    void removeRigidBody(RigidBody& body);

    void stepSimulation(float dt_s);
};

GF_NAMESPACE_END

#endif