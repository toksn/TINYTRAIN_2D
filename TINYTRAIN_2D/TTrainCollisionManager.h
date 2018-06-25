#pragma once
#include "CollisionManager.h"
#include "TTrain.h"

namespace tinytrain
{
	class TTrainCollisionManager : public tgf::collision::CollisionManager
	{
	public:
		TTrainCollisionManager();
		~TTrainCollisionManager();

		virtual void update() override;

		void addTrainToCollision(TTrain* train);
		virtual void removeFromCollision(void* obj) override;

		virtual std::vector<tgf::Entity*> tryCollideShape(tgf::collision::c2Shape shape, short collisionmask) override;
		virtual bool checkShapeForCollisions(tgf::collision::c2Shape shape, short collisionmask) override;
	protected:
		void tryCollideTrainObject(TTrain * train, collidingObject & obj);

		// trains, collide against everything and are handled special because they are not derived from TObstacle
		std::vector<TTrain*> trains_;
	};
}

