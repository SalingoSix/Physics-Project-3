#include "cPhysicsWorld.h"

namespace nPhysics
{
	cPhysicsWorld::~cPhysicsWorld()
	{

	}

	void cPhysicsWorld::TimeStep(float deltaTime)
	{
		const glm::vec3 GRAVITY = glm::vec3(0.0f, -2.0f, 0.0f);

		//SOFT BODY PHYSICS
		for (int index = 0; index < mVecSoftBodies.size(); index++)
		{	//At this point, it's only one body
			cSoftBody* softBody = mVecSoftBodies[index];
			//Go through each node one at a time, treat them each as a sphere rigid body
			for (int i = 0; i < softBody->numNodes(); i++)
			{
				//We will apply gravity, plus any collision forces
				glm::vec3 theForce = GRAVITY;
				glm::vec3 nodePos;
				softBody->getNodePosition(i, nodePos);
				float nodeRadius = 0.1f;
				for (int indexEO = 0; indexEO != mVecRigidBodies.size(); indexEO++)
				{
					cRigidBody* pOtherObject = mVecRigidBodies[indexEO];
					switch (pOtherObject->mShape->getShapeType())
					{
					case SHAPE_SPHERE:
						// 
						float otherRadius;
						glm::vec3 otherPos;
						pOtherObject->getShape()->sphereGetRadius(otherRadius);
						pOtherObject->getPosition(otherPos);
						if (PenetrationTestSphereSphere(nodeRadius, nodePos, otherRadius, otherPos))
						{
							float m1, m2, x1, x2;
							glm::vec3 v1temp, v1, v2, v1x, v2x, v1y, v2y, x(nodePos - otherPos);

							x = glm::normalize(x);
							v1 = softBody->getNodeVelocity(i);
							x1 = glm::dot(x, v1);
							v1x = x * x1;
							v1y = v1 - v1x;
							m1 = 0.01f; //We can assume at this point that all nodes have mass of 0.1

							x = -x;

							v2 = pOtherObject->mVelocity;
							x2 = glm::dot(x, v2);
							v2x = x * x2;
							v2y = v2 - v2x;
							m2 = pOtherObject->mMass;

							glm::vec3 newVel = glm::vec3(v1x*(m1 - m2) / (m1 + m2) + v2x * (2 * m2) / (m1 + m2) + v1y);
							softBody->setNodeVelocity(i, newVel);
							pOtherObject->mVelocity = glm::vec3(v1x*(2 * m1) / (m1 + m2) + v2x * (m2 - m1) / (m1 + m2) + v2y);
							pOtherObject->mPosition = pOtherObject->lastSafePos;
						}
						break;

					case SHAPE_PLANE:
						glm::vec3 planeBottomCorner, planeTopCorner;
						pOtherObject->getShape()->planeGetBottomCorner(planeBottomCorner);
						pOtherObject->getShape()->planeGetTopCorner(planeTopCorner);

						glm::vec3 vertexA = planeBottomCorner;
						glm::vec3 vertexB = planeTopCorner;
						glm::vec3 vertexC, vertexD;
						if (pOtherObject->getShape()->planeGetNormal() == 1)
						{	//The x value on the plane remains constant
							vertexC = glm::vec3(planeBottomCorner.x, planeBottomCorner.y, planeTopCorner.z);
							vertexD = glm::vec3(planeBottomCorner.x, planeTopCorner.y, planeBottomCorner.z);
						}
						else if (pOtherObject->getShape()->planeGetNormal() == 2)
						{	//The y value on the plane remains constant
							vertexC = glm::vec3(planeBottomCorner.x, planeBottomCorner.y, planeTopCorner.z);
							vertexD = glm::vec3(planeTopCorner.x, planeBottomCorner.y, planeBottomCorner.z);
						}
						else//(pOtherObject->getShape()->planeGetNormal() == 3)
						{	//The z value on the plane remains constant
							vertexC = glm::vec3(planeBottomCorner.x, planeTopCorner.y, planeBottomCorner.z);
							vertexD = glm::vec3(planeTopCorner.x, planeBottomCorner.y, planeBottomCorner.z);
						}

						glm::vec3 closestPointA = ClosestPtPointTriangle(nodePos, vertexA, vertexB, vertexC);
						float distanceA = glm::distance(nodePos, closestPointA);

						glm::vec3 closestPointB = ClosestPtPointTriangle(nodePos, vertexA, vertexB, vertexD);
						float distanceB = glm::distance(nodePos, closestPointB);

						if (distanceA <= nodeRadius || distanceB <= nodeRadius)
						{	//We must reset the node's velocity to 0 in the direction of the wall
							//As well as set a variable to make sure it does not move through the wall
							//as a result of being pulled on by any other nodes
							if (pOtherObject->getShape()->planeGetNormal() == 1)
							{
								glm::vec3 testCorner;
								pOtherObject->getShape()->planeGetBottomCorner(testCorner);
								if (testCorner.x > 0)
									softBody->wallSplat(i, -1);
								else
									softBody->wallSplat(i, 1);
								theForce.x = 0.0f;
							}
							else if (pOtherObject->getShape()->planeGetNormal() == 2)
							{
								theForce.y = 0.0f;
								softBody->wallSplat(i, 2);
							}
							else
							{
								glm::vec3 testCorner;
								pOtherObject->getShape()->planeGetBottomCorner(testCorner);
								if (testCorner.z > 0)
									softBody->wallSplat(i, -3);
								else
									softBody->wallSplat(i, 3);
								theForce.z = 0.0f;
							}

							break;
						}
						break;
					}
				}
				softBody->applyForce(i, theForce, deltaTime);
			}
		}

		//RIGID BODY PHYSICS
		for (int index = 0; index < mVecRigidBodies.size(); index++)
		{
			cRigidBody* pCurGO = mVecRigidBodies[index];

			// Explicit Euler  (RK4) 

			if (pCurGO->mShape->getShapeType() == SHAPE_SPHERE)
			{
				glm::vec3 theForce = GRAVITY;
				float thisRadius;
				pCurGO->getShape()->sphereGetRadius(thisRadius);
				if (pCurGO->mVelocity.y > -0.02f && pCurGO->mVelocity.y < 0.02f && pCurGO->mPosition.y < thisRadius)
				{
					theForce = glm::vec3(0.0f);
					pCurGO->mVelocity.y = 0.0f;
					if (pCurGO->mVelocity.x < 0.0f)
					{
						pCurGO->mVelocity.x += 0.1f;
					}
					else if (pCurGO->mVelocity.x > 0.0f)
					{
						pCurGO->mVelocity.x -= 0.1f;
					}
					if (pCurGO->mVelocity.z < 0.0f)
					{
						pCurGO->mVelocity.z += 0.1f;
					}
					else if (pCurGO->mVelocity.z > 0.0f)
					{
						pCurGO->mVelocity.z -= 0.1f;
					}
				}
				RK4(pCurGO->mPosition, pCurGO->mVelocity, (pCurGO->mAccel + ((float)deltaTime * GRAVITY) + ((float)deltaTime * pCurGO->appliedForce)), deltaTime);
				RK4(pCurGO->mPosition, pCurGO->mVelocity, pCurGO->mAccel, deltaTime);
				pCurGO->appliedForce = glm::vec3{ 0.0f };

			}	

			switch (pCurGO->mShape->getShapeType())
			{
			case SHAPE_SPHERE:
				// Compare this to EVERY OTHER object in the scene
				bool safeFrame = true;
				for (int indexEO = 0; indexEO != mVecRigidBodies.size(); indexEO++)
				{
					// Don't test for myself
					if (index == indexEO)
						continue;	// It's me!! 

					cRigidBody* pOtherObject = mVecRigidBodies[indexEO];
					// Is Another object
					switch (pOtherObject->mShape->getShapeType())
					{
					case SHAPE_SPHERE:
						// 
						float rad1, rad2, t;
						pCurGO->getShape()->sphereGetRadius(rad1);
						pOtherObject->getShape()->sphereGetRadius(rad2);
						if (PenetrationTestSphereSphere(pCurGO, pOtherObject))
						{
							safeFrame = false;
							pCurGO->mPosition = pCurGO->lastSafePos;
							float m1, m2, x1, x2;
							glm::vec3 v1temp, v1, v2, v1x, v2x, v1y, v2y, x(pCurGO->mPosition - pOtherObject->mPosition);

							x = glm::normalize(x);
							v1 = pCurGO->mVelocity;
							x1 = glm::dot(x, v1);
							v1x = x * x1;
							v1y = v1 - v1x;
							m1 = pCurGO->mMass; //Mass of the sphere. Set mass of all spheres to 1, just to simplify...

							x = -x;

							v2 = pOtherObject->mVelocity;
							x2 = glm::dot(x, v2);
							v2x = x * x2;
							v2y = v2 - v2x;
							m2 = pOtherObject->mMass;

							pCurGO->mVelocity = glm::vec3(v1x*(m1 - m2) / (m1 + m2) + v2x*(2 * m2) / (m1 + m2) + v1y) * 0.95f;
							pOtherObject->mVelocity = glm::vec3(v1x*(2 * m1) / (m1 + m2) + v2x*(m2 - m1) / (m1 + m2) + v2y) * 0.95f;
						}
						break;

					case SHAPE_PLANE:
						glm::vec3 planeBottomCorner, planeTopCorner;
						pOtherObject->getShape()->planeGetBottomCorner(planeBottomCorner);
						pOtherObject->getShape()->planeGetTopCorner(planeTopCorner);

						glm::vec3 vertexA = planeBottomCorner;
						glm::vec3 vertexB = planeTopCorner;
						glm::vec3 vertexC, vertexD;
						if (pOtherObject->getShape()->planeGetNormal() == 1)
						{	//The x value on the plane remains constant
							vertexC = glm::vec3(planeBottomCorner.x, planeBottomCorner.y, planeTopCorner.z);
							vertexD = glm::vec3(planeBottomCorner.x, planeTopCorner.y, planeBottomCorner.z);
						}
						else if (pOtherObject->getShape()->planeGetNormal() == 2)
						{	//The y value on the plane remains constant
							vertexC = glm::vec3(planeBottomCorner.x, planeBottomCorner.y, planeTopCorner.z);
							vertexD = glm::vec3(planeTopCorner.x, planeBottomCorner.y, planeBottomCorner.z);
						}
						else//(pOtherObject->getShape()->planeGetNormal() == 3)
						{	//The z value on the plane remains constant
							vertexC = glm::vec3(planeBottomCorner.x, planeTopCorner.y, planeBottomCorner.z);
							vertexD = glm::vec3(planeTopCorner.x, planeBottomCorner.y, planeBottomCorner.z);
						}

						glm::vec3 spherePos = pCurGO->mPosition;

						glm::vec3 closestPointA = ClosestPtPointTriangle(spherePos, vertexA, vertexB, vertexC);
						float distanceA = glm::distance(pCurGO->mPosition, closestPointA);

						glm::vec3 closestPointB = ClosestPtPointTriangle(spherePos, vertexA, vertexB, vertexD);
						float distanceB = glm::distance(pCurGO->mPosition, closestPointB);

						
						float curRadius;
						pCurGO->getShape()->sphereGetRadius(curRadius);
						if (distanceA <= curRadius || distanceB <= curRadius)
						{	//THIS IS A COLLISION
							safeFrame = false;
							pCurGO->mPosition = pCurGO->lastSafePos;
							glm::vec3 planeNormal = glm::vec3{ 0.0f };
							if (pOtherObject->getShape()->planeGetNormal() == 1)
							{
								planeNormal.x = 1.0f;								 
							}
							else if (pOtherObject->getShape()->planeGetNormal() == 2)
							{
								planeNormal.y = 1.0f;
							}
							else
							{
								planeNormal.z = 1.0f;
							}

							glm::vec3 reflection = 1.85f * planeNormal * (planeNormal * pCurGO->mVelocity);
							pCurGO->mVelocity -= reflection;

							break;
						}
						break;

					}//switch ( pOtherObject->typeOfObject )

				}
				if (safeFrame)
				{
					pCurGO->lastSafePos = pCurGO->mPosition;
				}
				break;
			};

			if (pCurGO->mPosition.y < -30.0f)
			{
				pCurGO->mPosition = glm::vec3{ 5.0f };
				pCurGO->mVelocity = glm::vec3(0.0f);
				pCurGO->mAccel = glm::vec3(0.0f);
			}

		}//for ( int index...
		return;
	}

	void cPhysicsWorld::AddRigidBody(iRigidBody* rigidBody)
	{
		cRigidBody* castBody = (cRigidBody*)rigidBody;
		if (castBody == NULL)
			return;

		for (int index = 0; index < mVecRigidBodies.size(); index++)
		{
			if (mVecRigidBodies[index] == castBody)
				return;
		}
		mVecRigidBodies.push_back(castBody);
		return;
	}

	void cPhysicsWorld::RemoveRigidBody(iRigidBody* rigidBody)
	{
		cRigidBody* castBody = (cRigidBody*)rigidBody;
		if (castBody == NULL)
			return;

		for (int index = 0; index < mVecRigidBodies.size(); index++)
		{
			if (mVecRigidBodies[index] == castBody)
			{
				mVecRigidBodies.erase(mVecRigidBodies.begin() + index);
				return;
			}
		}
		return;
	}

	void cPhysicsWorld::AddSoftBody(iSoftBody* newBody)
	{
		cSoftBody* castBody = (cSoftBody*)newBody;
		if (castBody == NULL)
			return;

		for (int index = 0; index < mVecSoftBodies.size(); index++)
		{
			if (mVecSoftBodies[index] == castBody)
				return;
		}
		mVecSoftBodies.push_back(castBody);
		return;
	}

	void cPhysicsWorld::RemoveSoftBody(iSoftBody* oldBody)
	{
		iSoftBody* castBody = (iSoftBody*)oldBody;
		if (castBody == NULL)
			return;

		for (int index = 0; index < mVecSoftBodies.size(); index++)
		{
			if (mVecSoftBodies[index] == castBody)
			{
				mVecSoftBodies.erase(mVecSoftBodies.begin() + index);
				return;
			}
		}
		return;
	}

	bool cPhysicsWorld::PenetrationTestSphereSphere(cRigidBody* pA, cRigidBody* pB)
	{
		float radiusA, radiusB;
		pA->mShape->sphereGetRadius(radiusA);
		pB->mShape->sphereGetRadius(radiusB);
		float totalRadii = radiusA + radiusB;

		// The Pythagorean distance 
		float distance = glm::distance(pA->mPosition, pB->mPosition);

		if (distance <= totalRadii)
		{
			return true;
		}

		return false;
	}

	bool cPhysicsWorld::PenetrationTestSphereSphere(float rA, glm::vec3 pA, float rB, glm::vec3 pB)
	{
		float totalRadii = rA + rB;

		// The Pythagorean distance 
		float distance = glm::distance(pA, pB);

		if (distance <= totalRadii)
		{
			return true;
		}

		return false;
	}

	int cPhysicsWorld::intersect_moving_sphere_sphere(
		const glm::vec3& c0, float r0, const glm::vec3& v0,
		const glm::vec3& c1, float r1, const glm::vec3& v1, float& t)
	{
		glm::vec3 s = c1 - c0;  // sphere center separation
		glm::vec3 v = v1 - v0;  // relative motion of sphere 1 w.r.t. stationary s0
		float r = r1 + r0; // sum of sphere radii
		float c = dot(s, s) - r * r;
		if (c < 0.f)
		{
			// spheres initially overlapping, exit early
			t = 0.f;
			return -1;
		}
		float a = dot(v, v);
		if (a < FLT_EPSILON) return 0; // spheres not moving relative to eachother
		float b = dot(v, s);
		if (b >= 0.f) return 0; // spheres not moving towards eachother
		float d = b * b - a * c;
		if (d < 0.f) return 0;  // no real root, so spheres do not intersect

		t = (-b - sqrt(d)) / a;
		return t < 1.f ? 1 : 0;
	}

	// c : sphere center
	// r : sphere radius
	// v : sphere movement
	// n : plane normal
	// d : plane dot product
	// t : output : [0,1] collision
	// q : output : collision point, where the sphere first contacts the plane
	// returns true/false whether the sphere collides with the plane in [c, c + v]
	int cPhysicsWorld::intersect_moving_sphere_plane(const glm::vec3& c, float r, const glm::vec3& v, const glm::vec3& n, float d, float& t, glm::vec3& q)
	{
		float dist = dot(n, c) - d;
		if (abs(dist) <= r)
		{
			// sphere is already overlapping the plane.
			// set time of intersection to 0 and q to sphere center
			t = 0.f;
			q = c;
			return -1;
		}
		else
		{
			float denom = dot(n, v);
			if (denom * dist >= 0.f)
			{
				// no intersection as sphere moving parallel to or away from plane
				return 0;
			}
			else
			{
				// sphere is moving towards the plane
				// use +r in computations if sphere in front of plane, else -r
				float rad = dist > 0.f ? r : -r;
				t = (r - dist) / denom;
				q = c + (v * t) - (n * rad);
				return t <= 1.f ? 1 : 0;
			}
		}
	}

	glm::vec3 cPhysicsWorld::ClosestPtPointTriangle(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c)
	{
		// Check if P in vertex region outside A
		glm::vec3 ab = b - a;
		glm::vec3 ac = c - a;
		glm::vec3 ap = p - a;
		float d1 = glm::dot(ab, ap);
		float d2 = glm::dot(ac, ap);
		if (d1 <= 0.0f && d2 <= 0.0f) return a; // barycentric coordinates (1,0,0)

												// Check if P in vertex region outside B
		glm::vec3 bp = p - b;
		float d3 = glm::dot(ab, bp);
		float d4 = glm::dot(ac, bp);
		if (d3 >= 0.0f && d4 <= d3) return b; // barycentric coordinates (0,1,0)

											  // Check if P in edge region of AB, if so return projection of P onto AB
		float vc = d1*d4 - d3*d2;
		if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
			float v = d1 / (d1 - d3);
			return a + v * ab; // barycentric coordinates (1-v,v,0)
		}

		// Check if P in vertex region outside C
		glm::vec3 cp = p - c;
		float d5 = glm::dot(ab, cp);
		float d6 = glm::dot(ac, cp);
		if (d6 >= 0.0f && d5 <= d6) return c; // barycentric coordinates (0,0,1)

											  // Check if P in edge region of AC, if so return projection of P onto AC
		float vb = d5*d2 - d1*d6;
		if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
			float w = d2 / (d2 - d6);
			return a + w * ac; // barycentric coordinates (1-w,0,w)
		}

		// Check if P in edge region of BC, if so return projection of P onto BC
		float va = d3*d6 - d5*d4;
		if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
			float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
			return b + w * (c - b); // barycentric coordinates (0,1-w,w)
		}

		// P inside face region. Compute Q through its barycentric coordinates (u,v,w)
		float denom = 1.0f / (va + vb + vc);
		float v = vb * denom;
		float w = vc * denom;
		return a + ab * v + ac * w; // = u*a + v*b + w*c, u = va * denom = 1.0f - v - w
	}

	sRK4State cPhysicsWorld::RK4Eval(const sRK4State &state, float dt, const sRK4State &derivative)
	{
		sRK4State s;
		s.Position = state.Position + derivative.Position * dt;
		s.Velocity = state.Velocity + derivative.Velocity * dt;

		sRK4State dS;
		dS.Position = s.Velocity;
		dS.Velocity = state.Acceleration;
		return dS;
	}

	void cPhysicsWorld::RK4(glm::vec3 &pos, glm::vec3 &vel, glm::vec3 &acc, float dt)
	{
		float halfDt = dt * 0.5f;
		sRK4State state(pos, vel, acc);
		sRK4State a = RK4Eval(state, 0.0f, sRK4State());
		sRK4State b = RK4Eval(state, halfDt, a);
		sRK4State c = RK4Eval(state, halfDt, b);
		sRK4State d = RK4Eval(state, dt, c);

		glm::vec3 rkVel = (a.Position + ((b.Position + c.Position) * 2.0f) + d.Position); // *(1.0f / 5.0f);
		glm::vec3 rkAcc = (a.Velocity + ((b.Velocity + c.Velocity) * 2.0f) + d.Velocity); // *(1.0f / 5.0f);

		pos += rkVel * dt;
		vel += rkAcc * dt;
	}
}