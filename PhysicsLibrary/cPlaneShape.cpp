#include "cPlaneShape.h"

#include <glm/geometric.hpp>

namespace nPhysics
{
	cPlaneShape::cPlaneShape(glm::vec3 cornerA, glm::vec3 cornerB)
		: iShape(SHAPE_PLANE)
	{
		bottomCorner = cornerA;
		topCorner = cornerB;

		if (topCorner.x == bottomCorner.x)
		{
			normalPlane = 1;
			normalVec = glm::vec3(1.0f, 0.0f, 0.0f);
		}
		else if (topCorner.y == bottomCorner.y)
		{
			normalPlane = 2;
			normalVec = glm::vec3(0.0f, 1.0f, 0.0f);
		}
		else
		{
			normalPlane = 3;
			normalVec = glm::vec3(0.0f, 0.0f, 1.0f);
		}
		this->planeConst = glm::dot(topCorner, bottomCorner);
	}

	cPlaneShape::~cPlaneShape()
	{

	}

	int cPlaneShape::planeGetNormal()
	{
		return normalPlane;
	}
	bool cPlaneShape::planeGetBottomCorner(glm::vec3 &corner)
	{
		corner = bottomCorner;
		return true;
	}
	bool cPlaneShape::planeGetTopCorner(glm::vec3 &corner)
	{
		corner = topCorner;
		return true;
	}
	bool cPlaneShape::planeGetNormalVec(glm::vec3 &normal)
	{
		normal = this->normalVec;
		return true;
	}
	bool cPlaneShape::planeGetPlaneConst(float &special)
	{
		special = this->planeConst;
		return true;
	}
}