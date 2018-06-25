#pragma once
#include "Entity.h"
//#include "tinyc2.h"
#include "CollisionManager.h"

namespace tgf
{
	namespace collision
	{
		class CollisionEntity : public Entity
		{
		public:
			virtual c2Shape getCollisionShape() = 0;
			//virtual void updateCollisionShape() = 0;
		};
	}
}