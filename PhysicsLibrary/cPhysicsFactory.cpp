#include "cPhysicsFactory.h"
#include "cPhysicsWorld.h"
#include "cRigidBody.h"
#include "cSoftBody.h"
#include "cSphereShape.h"
#include "cPlaneShape.h"

namespace nPhysics
{
	cPhysicsFactory::~cPhysicsFactory()
	{

	}

	iPhysicsWorld* cPhysicsFactory::CreateWorld()
	{
		return new cPhysicsWorld();
	}

	iRigidBody* cPhysicsFactory::CreateRigidBody(const sRigidBodyDesc &bodyDesc, iShape* shape)
	{
		return new cRigidBody(bodyDesc, shape);
	}

	iSoftBody* cPhysicsFactory::CreateSoftBody(sSoftBodyDesc &desc)
	{
		return new cSoftBody(desc);
	}

	iShape* cPhysicsFactory::CreateSphere(float radius)
	{
		return new cSphereShape(radius);
	}

	iShape* cPhysicsFactory::CreatePlane(const glm::vec3 cornerA, const glm::vec3 cornerB)
	{
		return new cPlaneShape(cornerA, cornerB);
	}
}