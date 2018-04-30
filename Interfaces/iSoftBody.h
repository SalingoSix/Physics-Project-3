#ifndef _HG_iSoftBody_
#define _HG_iSoftBody_

#include <vector>
#include <glm/vec3.hpp>
#include "iCollisionBody.h"
#include "eBodyType.h"

namespace nPhysics
{
	struct sSoftBodyDesc
	{
		std::vector<glm::vec3> vertices;
		std::vector<int> triangulatedIndices;
		std::vector<int> staticIndices;
	};

	class iSoftBody : public iCollisionBody
	{
	public:
		virtual bool getAABB(glm::vec3 &minBoundsOut, glm::vec3 &maxBoundsOut) = 0;
		virtual bool getNodePosition(int index, glm::vec3 &nodePositionOut) = 0;
		virtual int numNodes() = 0;

	protected:
		iSoftBody() : iCollisionBody(SOFT_BODY) {}
	};
}

#endif