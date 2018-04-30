#ifndef _HG_iRigidBody_
#define _HG_iRigidBody_

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "iShape.h"
#include "iCollisionBody.h"
#include "eBodyType.h"

namespace nPhysics
{
	struct sRigidBodyDesc
	{
		glm::vec3 Position;
		glm::vec3 Rotation;
		glm::vec3 Velocity;
		glm::vec3 Accel;
		float Mass;

		sRigidBodyDesc()
		{
			Position = glm::vec3(0.0f, 0.0f, 0.0f);
			Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
			Velocity = glm::vec3(0.0f, 0.0f, 0.0f);
			Accel = glm::vec3(0.0f, 0.0f, 0.0f);
			Mass = 1.0f;
		}
	};

	class iRigidBody : public iCollisionBody
	{
	public:
		virtual ~iRigidBody() {}

		virtual iShape* getShape() = 0;

		virtual void getTransform(glm::mat4 &transform) = 0;
		virtual void getPosition(glm::vec3 &position) = 0;
		virtual void getRotataion(glm::vec3 &rotation) = 0;
		virtual void getLastSafePos(glm::vec3 &safe) = 0;
		virtual void applyForce(glm::vec3 force) = 0;

	protected:
		iRigidBody() : iCollisionBody(RIGID_BODY) {}
	};
}


#endif
