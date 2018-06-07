#include "TCar.h"
#include "GameState_Running.h"
#include "TRoadNavComponent.h"

namespace tinytrain
{
	TCar::TCar(GameState_Running * gs, bool wintrigger) : TObstacle(gs, wintrigger)
	{
	}
	TCar::~TCar()
	{
	}
	void TCar::onUpdate(float deltaTime)
	{
		TObstacle::onUpdate(deltaTime);

		// check in front of the car
		tgf::collision::c2Shape s;
		s.type_ = C2_TYPE::C2_POLY;
		
		//s.type_ = C2_TYPE::C2_AABB;
		//c2AABB rect;
		//rect.min = { drawable_->getPosition().x- drawable_->getOrigin().x, drawable_->getPosition().y - drawable_->getOrigin().y};
		//rect.max = rect.min;
		//rect.max.x += drawable_->getSize().x;
		//rect.max.y += drawable_->getSize().y;
		//s.shape_ = &rect;
		
		s.shape_ = &collisionShape_;
		unsigned int mask = 0;
		mask |= TTrainCollisionManager::CollisionCategory::DYNAMIC_CATEGORY_1;
		if (gs_->getCollisionManager()->checkShapeForCollisions(s, mask))
			navi_->speed_ = 0.0f;
		else
			navi_->speed_ = vmax_;
	}
}
