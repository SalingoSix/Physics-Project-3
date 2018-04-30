#include "cSoftBody.h"

namespace nPhysics
{
	cSoftBody::cSpring::cSpring(cNode* nodeA, cNode* nodeB)
	{
		NodeA = nodeA;
		NodeB = nodeB;
		//I've been tweaking this. This value seems fine enough
		SpringConstantK = 10.0f;
		damperConstant = 2.0f;
		RestingSeparation = glm::distance(nodeA->Position, nodeB->Position);
		CurrentSeparation = RestingSeparation;
	}

	cSoftBody::cSpring::cSpring(cNode* nodeA, cNode* nodeB, float constant)
	{
		NodeA = nodeA;
		NodeB = nodeB;
		//I've been tweaking this. This value seems fine enough
		SpringConstantK = constant;
		damperConstant = 2.0f;
		RestingSeparation = glm::distance(nodeA->Position, nodeB->Position);
		CurrentSeparation = RestingSeparation;
	}

	cSoftBody::cNode* cSoftBody::cSpring::getOther(cNode* thisNode)
	{
		if (thisNode == NodeA)
		{
			return NodeB;
		}
		else if (thisNode == NodeB)
		{
			return NodeA;
		}
		else
			return NULL;
	}

	cSoftBody::cNode::cNode()
	{
		Mass = 0.1f;
		Radius = 0.1f;
		Position = glm::vec3(0.0f);
		Velocity = glm::vec3(0.0f);
		Acceleration = glm::vec3(0.0f);
		isStatic = false;
		splatValue = 0;
	}

	bool cSoftBody::cNode::HasNeighbour(cNode* other)
	{
		for (int i = 0; i < Springs.size(); i++)
		{
			cSoftBody::cSpring* thisSpring = Springs[i];
			cSoftBody::cNode* otherNode = thisSpring->getOther(this);
			if (otherNode == other)
				return true;
		}
		return false;
	}

	cSoftBody::cSoftBody(sSoftBodyDesc &desc)
	{
		for (int i = 0; i < desc.vertices.size(); i++)
		{
			cNode* newNode = new cNode();
			newNode->Position = desc.vertices[i];
			newNode->ID = i;
			mNodes.push_back(newNode);
		}

		int dimensionX;
		for (int i = 0; i < desc.staticIndices.size(); i++)
		{
			int staticOne = desc.staticIndices[i];
			dimensionX = staticOne;
			mNodes[staticOne]->isStatic = true;
		}
		float constant = 17.0f;

		for (int i = 0; i < desc.triangulatedIndices.size(); i += 3)
		{
			if (i % dimensionX == 0)
			{
				constant -= 1.0f;
			}

			int first, second, third;
			first = desc.triangulatedIndices[i];
			second = desc.triangulatedIndices[i + 1];
			third = desc.triangulatedIndices[i + 2];
			cNode *firstNode, *secondNode, *thirdNode;
			firstNode = mNodes[first];
			secondNode = mNodes[second];
			thirdNode = mNodes[third];

			//Checks if two nodes share a spring before it makes another instance of that spring
			if (!firstNode->HasNeighbour(secondNode))
			{
				cSpring* firstSpring = new cSpring(firstNode, secondNode);
				mSprings.push_back(firstSpring);
				firstNode->Springs.push_back(firstSpring);
				secondNode->Springs.push_back(firstSpring);
			}

			if (!firstNode->HasNeighbour(thirdNode))
			{
				cSpring* secondSpring = new cSpring(firstNode, thirdNode);
				mSprings.push_back(secondSpring);
				firstNode->Springs.push_back(secondSpring);
				thirdNode->Springs.push_back(secondSpring);
			}

			if (!secondNode->HasNeighbour(thirdNode))
			{
				cSpring* thirdSpring = new cSpring(secondNode, thirdNode);
				mSprings.push_back(thirdSpring);
				secondNode->Springs.push_back(thirdSpring);
				thirdNode->Springs.push_back(thirdSpring);
			}
		}
	}

	cSoftBody::~cSoftBody()
	{

	}

	bool cSoftBody::getAABB(glm::vec3 &minBoundsOut, glm::vec3 &maxBoundsOut)
	{
		return false;
	}

	bool cSoftBody::getNodePosition(int index, glm::vec3 &nodePositionOut)
	{
		if (mNodes.size() <= index)
			return false;
		nodePositionOut = mNodes[index]->Position;
		return true;
	}

	int cSoftBody::numNodes()
	{
		return mNodes.size();
	}

	void cSoftBody::applyForce(glm::vec3 theForce, float deltaTime)
	{
		for (int index = 0; index < mNodes.size(); index++)
		{
			cNode* thisNode = mNodes[index];
			if (thisNode->isStatic)
				continue;

			//Apply the force (gravity plus any other applied forces)
			glm::vec3 deltaVeloctiy = deltaTime * thisNode->Acceleration + deltaTime * theForce;
			thisNode->Velocity += deltaVeloctiy;
			//Update position based on the new velocity
			glm::vec3 deltaPosition = deltaTime * thisNode->Velocity;
			thisNode->Position += deltaPosition;

			for (int springIndex = 0; springIndex < thisNode->Springs.size(); springIndex++)
			{
				cSpring* thisSpring = thisNode->Springs[springIndex];
				cNode* thatNode = thisSpring->getOther(thisNode);

				//A vector representing what the spring between these two should look like
				glm::vec3 springVector = thatNode->Position - thisNode->Position;

				//The length of this vector
				float pairDistance = glm::distance(thisNode->Position, thatNode->Position);

				//The difference between the length now, and when the spring is resting
				float springDisplacement = pairDistance - thisSpring->RestingSeparation;

				glm::vec3 normalizedSpring = springVector / pairDistance;

				//Our magic equation: (F = -kx) in the direction of the spring's normal
				glm::vec3 restoringForce = normalizedSpring * springDisplacement * thisSpring->SpringConstantK;

				glm::vec3 diffVelocity = thatNode->Velocity - thisNode->Velocity;

				float damperForce = glm::dot(normalizedSpring, diffVelocity);

				float dampConst = 2.5f;
				glm::vec3 dForce = normalizedSpring * damperForce * dampConst;

				restoringForce += dForce;

				deltaVeloctiy = deltaTime * thisNode->Acceleration + deltaTime * restoringForce;
				thisNode->Velocity += deltaVeloctiy;
				//Update position based on the new velocity
				glm::vec3 deltaPosition = deltaTime * thisNode->Velocity;
				thisNode->Position += deltaPosition;
			}

		}
	}

	void cSoftBody::applyForce(int index, glm::vec3 theForce, float deltaTime)
	{
		cNode* thisNode = mNodes[index];
		if (thisNode->isStatic)
			return;

		//Apply the force (gravity plus any other applied forces)
		glm::vec3 deltaVeloctiy = deltaTime * thisNode->Acceleration + deltaTime * theForce;
		thisNode->Velocity += deltaVeloctiy;
		//Update position based on the new velocity
		glm::vec3 deltaPosition = deltaTime * thisNode->Velocity;
		thisNode->Position += deltaPosition;
		
		glm::vec3 restoringForce = glm::vec3(0.0f);
		//Go through each of the node's springs
		for (int springIndex = 0; springIndex < thisNode->Springs.size(); springIndex++)
		{
			cSpring* thisSpring = thisNode->Springs[springIndex];
			cNode* thatNode = thisSpring->getOther(thisNode);

			//A vector representing what the spring between these two should look like
			glm::vec3 springVector = thatNode->Position - thisNode->Position;

			//The length of this vector
			float pairDistance = glm::distance(thisNode->Position, thatNode->Position);

			//The difference between the length now, and when the spring is resting
			float springDisplacement = pairDistance - thisSpring->RestingSeparation;

			glm::vec3 normalizedSpring = springVector / pairDistance;

			//Our magic equation: (F = -kx) in the direction of the spring's normal
			restoringForce = normalizedSpring * springDisplacement * thisSpring->SpringConstantK;

			glm::vec3 diffVelocity = thatNode->Velocity - thisNode->Velocity;

			//Even more magical damping value: (-bv)
			glm::vec3 dForce = diffVelocity * thisSpring->damperConstant;

			//Apply the damping force to the spring force
			restoringForce += dForce;

			//Check if the node hit a wall this frame, and reset its velocity in the direction of that wall
			if (thisNode->splatValue == 1)
			{
				if (restoringForce.x < 0.0f)
					restoringForce.x = 0.0f;
			}
			else if (thisNode->splatValue == -1)
			{
				if (restoringForce.x > 0.0f)
					restoringForce.x = 0.0f;
			}

			else if (thisNode->splatValue == 2)
			{
				if (restoringForce.y < 0.0f)
					restoringForce.y = 0.0f;
			}

			else if (thisNode->splatValue == 3)
			{
				if (restoringForce.z < 0.0f)
					restoringForce.z = 0.0f;
			}
			else if (thisNode->splatValue == -3)
			{
				if (restoringForce.z > 0.0f)
					restoringForce.z = 0.0f;
			}
			deltaVeloctiy = deltaTime * thisNode->Acceleration + deltaTime * restoringForce;
			thisNode->Velocity += deltaVeloctiy;
			//Update position based on the new velocity
			deltaPosition = deltaTime * thisNode->Velocity;
			thisNode->Position += deltaPosition;
		}

		//Although hilarious when forgotten, it's important to reset the splat value
		thisNode->splatValue = 0;
	}

	//The purpose of this function is to reset velocity to 0 when a node hits a wall
	void cSoftBody::wallSplat(int index, int normal)
	{
		cNode* thisNode = mNodes[index];
		if (normal == 1 || normal == -1)
		{
			thisNode->Velocity.x = 0.0f;
			thisNode->splatValue = normal;
		}
		else if (normal == 2)
		{
			thisNode->Velocity.y = 0.0f;
			thisNode->splatValue = normal;
		}
		else if (normal == 3 || normal == -3)
		{
			thisNode->Velocity.z = 0.0f;
			thisNode->splatValue = normal;
		}
		else
		{
			//Something went wrong. Don't do this.
		}
		return;
	}

	glm::vec3 cSoftBody::getNodeVelocity(int index)
	{
		return mNodes[index]->Velocity;
	}

	void cSoftBody::setNodeVelocity(int index, glm::vec3 newVel)
	{
		mNodes[index]->Velocity = newVel;
	}
}