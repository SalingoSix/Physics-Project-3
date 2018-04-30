#ifndef _HG_iCollisionBody_
#define _HG_iCollisionBody_

#include "eBodyType.h"

namespace nPhysics
{
	class iCollisionBody
	{
	public:
		virtual ~iCollisionBody() {}

		eBodyType GetBodyType() { return this->mBodyType; }

	protected:
		iCollisionBody(eBodyType bodyType) : mBodyType(bodyType) {}
		iCollisionBody() {}
	private:
		eBodyType mBodyType;
	};
}

#endif