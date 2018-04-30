#include "cRigidBody.h"

namespace nPhysics
{
	cRigidBody::cRigidBody(const sRigidBodyDesc desc, iShape* shape)
	{
		mShape = shape;
		mPosition = desc.Position;
		lastSafePos = desc.Position;
		mVelocity = desc.Velocity;
		mRotation = desc.Rotation;
		mMass = desc.Mass;
	}
	
	cRigidBody::~cRigidBody()
	{

	}

	iShape* cRigidBody::getShape()
	{
		return mShape;
	}

	void cRigidBody::getTransform(glm::mat4 &transform)
	{
		transform = glm::mat4_cast(mRotation);
		transform[3][0] = mPosition.x;
		transform[3][1] = mPosition.y;
		transform[3][2] = mPosition.z;
		transform[3][3] = 1.f;
	}

	void cRigidBody::getPosition(glm::vec3 &position)
	{
		position = mPosition;
	}

	void cRigidBody::getRotataion(glm::vec3 &rotation)
	{
		rotation = glm::eulerAngles(mRotation);
	}

	void cRigidBody::getLastSafePos(glm::vec3 &safePos)
	{
		safePos = lastSafePos;
	}

	void cRigidBody::applyForce(glm::vec3 force)
	{
		appliedForce = force;
	}
}