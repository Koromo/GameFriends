#include "rigidbody.h"
#include "collisionshape.h"
#include "../foundation/exception.h"

GF_NAMESPACE_BEGIN

RigidBody::MyMotionState::MyMotionState(const btTransform& startTrans, const btTransform& centerOfMassOffset)
    : btDefaultMotionState(startTrans, centerOfMassOffset)
{
}

void RigidBody::MyMotionState::setWorldTransform(const btTransform& trans)
{
    btDefaultMotionState::setWorldTransform(trans);
    const auto body = reinterpret_cast<RigidBody*>(m_userPointer);
    if (body->inWorld() && body->listener_)
    {
        body->listener_->move(
            to<Vector3>(m_graphicsWorldTrans.getOrigin()),
            to<Quaternion>(m_graphicsWorldTrans.getRotation()));
    }
}

RigidBody::RigidBody(const std::shared_ptr<CollisionShape> shape, float mass)
    : body_()
    , motionState_()
    , shape_(shape)
    , listener_()
    , usrPtr_(nullptr)
{
    motionState_.m_userPointer = this;

    if (!shape->dynamic())
    {
        mass = 0;
    }

    auto& btShape = shape_->btShape();
    btVector3 inertia(0, 0, 0);
    if (mass > 0)
    {
        btShape.calculateLocalInertia(mass, inertia);
    }
    btRigidBody::btRigidBodyConstructionInfo ci(mass, &motionState_, &btShape, inertia);
    body_ = std::make_unique<btRigidBody>(ci);
    body_->setUserPointer(this);
}

bool RigidBody::inWorld() const
{
    return body_->isInWorld();
}

void RigidBody::setPhysicsListener(PhysicsListener* listener)
{
    listener_ = listener;
}

void RigidBody::setPosition(const Vector3& pos)
{
    check(!inWorld());
    auto trans = body_->getCenterOfMassTransform();
    trans.setOrigin(to<btVector3>(pos));
    body_->setCenterOfMassTransform(trans);
    motionState_.btDefaultMotionState::setWorldTransform(trans);
}

void RigidBody::setOrientation(const Quaternion& q)
{
    check(!inWorld());
    auto trans = body_->getCenterOfMassTransform();
    trans.setRotation(to<btQuaternion>(q));
    body_->setCenterOfMassTransform(trans);
    motionState_.btDefaultMotionState::setWorldTransform(trans);
}

void RigidBody::setLinearVelocity(const Vector3& vel)
{
    body_->setLinearVelocity(to<btVector3>(vel));
}

void RigidBody::setAngularVelocity(const Vector3& vel)
{
    body_->setAngularVelocity(to<btVector3>(vel));
}

void RigidBody::applyCentralForce(const Vector3& force)
{
    body_->applyCentralForce(to<btVector3>(force));
}

void RigidBody::activate(bool b)
{
    body_->activate(b);
}

void RigidBody::notifyCollision(RigidBody& collider)
{
    if (listener_)
    {
        listener_->collision(collider);
    }
}

void RigidBody::setUserPointer(void* p)
{
    usrPtr_ = p;
}

void* RigidBody::userPointer()
{
    return usrPtr_;
}

btRigidBody& RigidBody::btBody()
{
    return *body_;
}

GF_NAMESPACE_END