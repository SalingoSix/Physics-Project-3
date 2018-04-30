#ifndef _HG_iPhysicsFactory_
#define _HG_iPhysicsFactory_

#include "iRigidBody.h"
#include "iSoftBody.h"
#include "iShape.h"
#include "iPhysicsWorld.h"

namespace nPhysics
{
	class iPhysicsFactory
	{
	public:
		virtual ~iPhysicsFactory() {}

		virtual iPhysicsWorld* CreateWorld() = 0;

		virtual iRigidBody* CreateRigidBody(const sRigidBodyDesc &bodyDesc, iShape* shape) = 0;
		virtual iSoftBody* CreateSoftBody(sSoftBodyDesc &desc) = 0;

		virtual iShape* CreateSphere(float radius) = 0;
		virtual iShape* CreatePlane(const glm::vec3 cornerA, const glm::vec3 cornerB) = 0;
	};
}


#endif
