#ifndef _cGameObject_HG_
#define _cGameObject_HG_

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iRigidBody.h>
#include <iShape.h>

#include <string>

class cGameObject
{
public:
	cGameObject();		// constructor
	~cGameObject();		// destructor

	bool wireFrame;
	bool bIsUpdatedInPhysics;

	//Things pertaining to physics will be stored here.
	nPhysics::iRigidBody* myBody;

	glm::vec4 diffuseColour;
	std::string meshName;
};

#endif