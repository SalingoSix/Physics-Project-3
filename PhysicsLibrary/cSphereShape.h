#ifndef _HG_cSphereShape_
#define _HG_cSphereShape_

#include <glm/vec3.hpp>
#include <iShape.h>

namespace nPhysics
{
	class cSphereShape : public iShape
	{
	public:
		cSphereShape(float radius);

		virtual ~cSphereShape();

		virtual bool sphereGetRadius(float &radius);

	private:
		float mRadius;
	};
}

#endif
