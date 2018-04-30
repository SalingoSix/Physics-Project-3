#ifndef _HG_cRigidBody_
#define _HG_cRigidBody_

#include <iRigidBody.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace nPhysics
{
	class cPhysicsWorld;
	class cRigidBody : public iRigidBody
	{
	public:
		cRigidBody(const sRigidBodyDesc desc, iShape* shape);
		virtual ~cRigidBody();

		virtual iShape* getShape();

		virtual void getTransform(glm::mat4 &transform);
		virtual void getPosition(glm::vec3 &position);
		virtual void getRotataion(glm::vec3 &rotation);
		virtual void getLastSafePos(glm::vec3 &safe);
		virtual void applyForce(glm::vec3 force);

	private:
		friend class cPhysicsWorld;

		iShape* mShape;
		glm::vec3 mPosition;
		glm::vec3 lastSafePos;
		glm::vec3 mVelocity;
		glm::quat mRotation;
		glm::vec3 mAccel;
		glm::vec3 appliedForce;
		float mMass;
	};
}

#endif
