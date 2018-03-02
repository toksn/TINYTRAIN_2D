#include "CollisionEntity.h"

namespace tgf
{
	namespace collision
	{
		CollisionEntity::CollisionEntity()
		{
			collisionShape_ = std::make_unique<c2Poly>();
			if (collisionShape_)
			{
				updateCollisionShape();
			}
		}


		CollisionEntity::~CollisionEntity()
		{
		}

		c2Shape CollisionEntity::getCollisionShape()
		{
			c2Shape s;
			s.type_ = C2_POLY;
			s.shape_ = collisionShape_.get();
			return s;
		}
		void CollisionEntity::updateCollisionShape()
		{
			// this function is used to manually update the collisionshape after the tracked object was altered
			collisionShape_->count = 4;
		}
	}
}
