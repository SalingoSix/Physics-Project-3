#include "cGameObject.h"

cGameObject::cGameObject()
{
	this->wireFrame = false;

	bIsUpdatedInPhysics = false;

	this->diffuseColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	return;
}

cGameObject::~cGameObject()
{
	return;
}