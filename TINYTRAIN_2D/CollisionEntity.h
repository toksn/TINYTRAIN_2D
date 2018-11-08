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
			virtual c2Shape getCollisionShape() { return collisionShape_; };
			bool collisionUpdated = false;
		protected:
			tgf::collision::c2Shape collisionShape_;
			
		};
	}
}