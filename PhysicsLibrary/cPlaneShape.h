#ifndef _HG_cPlaneShape_
#define _HG_cPlaneShape_

#include <glm/vec3.hpp>
#include <iShape.h>

namespace nPhysics
{
	class cPlaneShape : public iShape
	{
	public:
		cPlaneShape(glm::vec3 cornerA, glm::vec3 cornerB);

		virtual ~cPlaneShape();

		virtual int planeGetNormal();
		virtual bool planeGetBottomCorner(glm::vec3 &corner);
		virtual bool planeGetTopCorner(glm::vec3 &corner);
		virtual bool planeGetNormalVec(glm::vec3 &normal);
		virtual bool planeGetPlaneConst(float &planeConst);

	private:
		glm::vec3 bottomCorner;
		glm::vec3 topCorner;
		glm::vec3 normalVec;
		float planeConst;
		int normalPlane; //1 for x, 2 for y, 3 for z
	};
}

#endif
