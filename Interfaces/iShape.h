#ifndef _HG_iShape_
#define _HG_iShape_

#include <glm/vec3.hpp>

namespace nPhysics
{
	enum eShapeType
	{
		SHAPE_SPHERE,
		SHAPE_PLANE
	};

	class iShape
	{
	public:
		virtual ~iShape() {}

		eShapeType getShapeType()
		{
			return shapeType;
		}

		virtual bool sphereGetRadius(float &radius)
		{
			return false;
		}

		virtual int planeGetNormal()
		{
			return -1;
		}

		virtual bool planeGetBottomCorner(glm::vec3 &corner)
		{
			return false;
		}

		virtual bool planeGetTopCorner(glm::vec3 &corner)
		{
			return false;
		}

		virtual bool planeGetNormalVec(glm::vec3 &normal)
		{
			return false;
		}

		virtual bool planeGetPlaneConst(float &planeConst)
		{
			return false;
		}

	protected:
		iShape(eShapeType type)
		{
			shapeType = type;
		}

	private:
		eShapeType shapeType;
	};
}

#endif
