#pragma once
//#include "Entity.h"
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "tinyc2.h"
#include "BroadPhase.h"

namespace tgf
{
	namespace collision
	{
		class CollisionEntity;
		//class BroadPhase_CategoryFilter;
		
		template<class T>
		using collisionCallbackFunc = void(T::*)(tgf::collision::CollisionEntity*);

		// obstacles, collide against everything in their mask when the other obj has its mask set to collide against it as well
		struct c2Shape
		{
			void* shape_;
			C2_TYPE type_;
		};


		class CollisionManager
		{
		public:
			CollisionManager();
			~CollisionManager();

			virtual void update();

						
			template<class T> void addToCollision(T* const object, void(T::* const on_enter)(CollisionEntity*), void(T::* const on_leave)(CollisionEntity*), CollisionCategory category = CollisionCategory::STATIC_CATEGORY_1, uint16_t collisionmask = (uint16_t)CollisionCategory::STATIC_CATEGORY_1)
			{
				collidingObject col;
				col.callback_enter = col.callback_leave = nullptr;
				if (on_enter)
					col.callback_enter = std::bind(on_enter, object, std::placeholders::_1);
				if (on_leave)
					col.callback_leave = std::bind(on_leave, object, std::placeholders::_1);
				col.obj = object;
				col.obj->collisionUpdated = true;
				col.collision_mask = collisionmask;
				col.collision_category = category;

				broadphase_->add(col);
			}

			virtual void removeFromCollision(void* obj);

			virtual std::vector<CollisionEntity*> tryCollideShape(c2Shape* shape, uint16_t collisionmask);
			virtual bool checkShapeForCollisions(c2Shape* shape, uint16_t collisionmask);
		protected:
			virtual void tryCollideObjects(collidingObject & obj1, collidingObject & obj2);

			std::unique_ptr<Broadphase> broadphase_;
		};
	}
}

