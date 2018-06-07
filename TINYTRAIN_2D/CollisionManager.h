#pragma once
#include "Entity.h"
#include <memory>
#include <functional>
#include "tinyc2.h"

namespace tgf
{
	namespace collision
	{
		class CollisionEntity;
		struct c2Shape
		{
			void* shape_;
			C2_TYPE type_;
		};

		template<class T>
		using collisionCallbackFunc = void(T::*)(tgf::Entity*);

		class CollisionManager
		{
		public:
			CollisionManager();
			~CollisionManager();

			virtual void update();

			enum CollisionCategory
			{
				STATIC_CATEGORY_1	= (1u << 0),
				STATIC_CATEGORY_2	= (1u << 1),
				STATIC_CATEGORY_3	= (1u << 2),
				STATIC_CATEGORY_4	= (1u << 3),
				STATIC_CATEGORY_5	= (1u << 4),
				DYNAMIC_CATEGORY_1	= (1u << 5),
				DYNAMIC_CATEGORY_2	= (1u << 6),
				DYNAMIC_CATEGORY_3	= (1u << 7),
				DYNAMIC_CATEGORY_4	= (1u << 8),
				DYNAMIC_CATEGORY_5	= (1u << 9), 
				OTHER_CATEGORY_1	= (1u << 10),
				OTHER_CATEGORY_2	= (1u << 11),
				OTHER_CATEGORY_3	= (1u << 12),
				OTHER_CATEGORY_4	= (1u << 13),
				OTHER_CATEGORY_5	= (1u << 14),
				OTHER_CATEGORY_6	= (1u << 15)
			};

			// obstacles, collide against everything in their mask when the other obj has its mask set to collide against it as well
			struct collidingObject
			{
				CollisionEntity* obj;
				std::function<void(Entity*)> callback_enter;
				std::function<void(Entity*)> callback_leave;
				short collision_mask;
				std::vector<Entity*> currentCollisions;
			};
			
			template<class T> void addToCollision(T* const object, void(T::* const on_enter)(Entity*), void(T::* const on_leave)(Entity*), CollisionCategory category = CollisionCategory::STATIC_CATEGORY_1, short collisionmask = (short)CollisionCategory::STATIC_CATEGORY_1)
			{
				collidingObject col;
				col.callback_enter = col.callback_leave = nullptr;
				if (on_enter)
					col.callback_enter = std::bind(on_enter, object, std::placeholders::_1);
				if (on_leave)
					col.callback_leave = std::bind(on_leave, object, std::placeholders::_1);
				col.obj = object;
				col.collision_mask = collisionmask;
				colliders_[category].push_back(col);
			}

			virtual void removeFromCollision(void* obj);

			std::vector<Entity*> findShapeCollisions(c2Shape shape, short collisionmask);
			bool checkShapeForCollisions(c2Shape shape, short collisionmask);
		protected:
			virtual void tryCollideObjects(collidingObject & obj1, collidingObject & obj2);

			// store function pointers to call when a collision did hit, mapped to categories
			std::map<CollisionCategory, std::vector<collidingObject>> colliders_;
		};
	}
}

