#include "cSphereShape.h"

namespace nPhysics
{
	cSphereShape::cSphereShape(float radius)
		: iShape(SHAPE_SPHERE)
	{
		mRadius = radius;
	}

	cSphereShape::~cSphereShape()
	{

	}

	bool cSphereShape::sphereGetRadius(float &radius)
	{
		radius = mRadius;
		return true;
	}
}
