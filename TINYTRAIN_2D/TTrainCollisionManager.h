#pragma once
#include <memory>
#include <functional>
#include "TTrain.h"
#include "tinyc2.h"

namespace tinytrain
{
	class TObstacle;
	struct c2Shape
	{
		void* shape_;
		C2_TYPE type_;
	};
	class TTrainCollisionManager
	{
	public:
		TTrainCollisionManager();
		~TTrainCollisionManager();

		virtual void update();

		enum class CollisionCategory
		{
			OBSTACLE_WIN		= 0x1,
			OBSTACLE_LOOSE		= 0x2
			//,  TRAIN			= 0x4
		};

		// obstacles, collide against trains and everything in their mask when the other obj has its mask set to collide against it
		struct collidingObject
		{
			TObstacle* obj;
			std::function<void(tgf::Entity*)> callback;
			short collision_mask;
		};

		void addToCollision(TObstacle* const object, void(TObstacle::* const mf)(tgf::Entity*), CollisionCategory category = CollisionCategory::OBSTACLE_LOOSE, short collisionmask = (short)CollisionCategory::OBSTACLE_LOOSE);
		void addToCollision(TTrain* train);

		void removeFromCollision(void* obj);

	protected:
		void tryCollideObjects(collidingObject & obj1, collidingObject & obj2);
		void tryCollideTrainObject(TTrain * train, collidingObject & obj);

		// store function pointers to call when a collision did hit, mapped to categories
		std::map<CollisionCategory, std::vector<collidingObject>> colliders_;
		// trains, collide against everything and are handled special because they are not derived from TObstacle
		std::vector<TTrain*> trains_;
	};
}

