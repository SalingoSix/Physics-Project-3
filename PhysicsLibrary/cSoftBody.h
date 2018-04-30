#ifndef _HG_cSoftBody_
#define _HG_cSoftBody_

#include <iSoftBody.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace nPhysics
{
	class cSoftBody : public iSoftBody
	{
		class cNode;
		class cSpring
		{
		public:
			cSpring(cNode* nodeA, cNode* nodeB);
			cSpring(cNode* nodeA, cNode* nodeB, float constant);

			cNode* getOther(cNode* thisNode);

			float RestingSeparation;
			float CurrentSeparation;
			float SpringConstantK;
			float damperConstant;
			cNode* NodeA;
			cNode* NodeB;
		};

		class cNode
		{
		public:
			cNode();

			bool HasNeighbour(cNode* other);
			std::vector<cSpring*> Springs;

			int ID;

			//A variable to indicate that the node has hit a wall this frame, as well as which wall it hit
			int splatValue;
			//Regular rigid body variables
			float Mass;
			float Radius;
			glm::vec3 Position;
			glm::vec3 Velocity;
			glm::vec3 Acceleration;
			bool isStatic;
		};

	public:
		cSoftBody(sSoftBodyDesc &desc);
		//Never actually implemented this one...
		virtual bool getAABB(glm::vec3 &minBoundsOut, glm::vec3 &maxBoundsOut);
		//Pulls the node's position. Used for drawing
		virtual bool getNodePosition(int index, glm::vec3 &nodePositionOut);
		//Returns the number of nodes
		virtual int numNodes();
		//Applies a single force to the entire soft body (currently unused)
		void applyForce(glm::vec3 theForce, float deltaTime);
		//Applies a force to a single node. Should cycle through all nodes within one frame
		void applyForce(int index, glm::vec3 theForce, float deltaTime);
		//Indicates the node has run into a wall, and ensures it doesn't go through it
		void wallSplat(int index, int normal);
		//Getting and setting velocity.
		glm::vec3 getNodeVelocity(int index);
		void setNodeVelocity(int index, glm::vec3 newVel);
		~cSoftBody();

	protected:
		std::vector<cNode*> mNodes;
		std::vector<cSpring*> mSprings;
	};
}

#endif