#ifndef _HG_cPhysicsWorld_
#define _HG_cPhysicsWorld_

#include <iPhysicsWorld.h>
#include <vector>
#include "cRigidBody.h"
#include "cSoftBody.h"

namespace nPhysics
{

	struct sRK4State
	{
		sRK4State(glm::vec3 a, glm::vec3 b, glm::vec3 c)
		{
			Position = a;
			Velocity = b;
			Acceleration = c;
		}
		sRK4State()
		{
			Position = glm::vec3{ 0.0f };
			Velocity = glm::vec3{ 0.0f };
			Acceleration = glm::vec3{ 0.0f };
		}
		glm::vec3 Position;
		glm::vec3 Velocity;
		glm::vec3 Acceleration;
	};

	class cPhysicsWorld : public iPhysicsWorld
	{
		virtual ~cPhysicsWorld();

		virtual void TimeStep(float deltaTime);

		virtual void AddRigidBody(iRigidBody* rigidBody);
		virtual void RemoveRigidBody(iRigidBody* rigidBody);
		virtual void AddSoftBody(iSoftBody* newBody);
		virtual void RemoveSoftBody(iSoftBody* oldBody);

	private:
		bool PenetrationTestSphereSphere(cRigidBody* pA, cRigidBody* pB);
		bool PenetrationTestSphereSphere(float rA, glm::vec3 pA, float rB, glm::vec3 pB);
		int intersect_moving_sphere_sphere(const glm::vec3& c0, float r0, const glm::vec3& v0, const glm::vec3& c1, float r1, const glm::vec3& v1, float& t);
		int intersect_moving_sphere_plane(const glm::vec3& c, float r, const glm::vec3& v, const glm::vec3& n, float d, float& t, glm::vec3& q);
		glm::vec3 ClosestPtPointTriangle(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c);
		sRK4State RK4Eval(const sRK4State &state, float dt, const sRK4State &derivative);
		void RK4(glm::vec3 &pos, glm::vec3 &vel, glm::vec3 &acc, float dt);
		std::vector<cRigidBody*> mVecRigidBodies;
		std::vector<cSoftBody*> mVecSoftBodies;
	};
}

#endif