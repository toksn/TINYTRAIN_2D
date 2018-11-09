#pragma once
#include <functional>
#include <vector>
#include <map>
namespace tgf
{
	namespace collision
	{
		class CollisionEntity;
		struct c2Shape;

		struct collidingObject
		{
			CollisionEntity* obj;
			std::function<void(CollisionEntity*)> callback_enter;
			std::function<void(CollisionEntity*)> callback_leave;
			uint16_t collision_mask;
			uint16_t collision_category;
			std::vector<CollisionEntity*> currentCollisions;
		};
		enum CollisionCategory
		{
			STATIC_CATEGORY_1 = (1u << 0),
			STATIC_CATEGORY_2 = (1u << 1),
			STATIC_CATEGORY_3 = (1u << 2),
			STATIC_CATEGORY_4 = (1u << 3),
			STATIC_CATEGORY_5 = (1u << 4),
			DYNAMIC_CATEGORY_1 = (1u << 5),
			DYNAMIC_CATEGORY_2 = (1u << 6),
			DYNAMIC_CATEGORY_3 = (1u << 7),
			DYNAMIC_CATEGORY_4 = (1u << 8),
			DYNAMIC_CATEGORY_5 = (1u << 9),
			OTHER_CATEGORY_1 = (1u << 10),
			OTHER_CATEGORY_2 = (1u << 11),
			OTHER_CATEGORY_3 = (1u << 12),
			OTHER_CATEGORY_4 = (1u << 13),
			OTHER_CATEGORY_5 = (1u << 14),
			OTHER_CATEGORY_6 = (1u << 15)
		};

		class BroadPhase_CategoryFilter
		{
		public:
			BroadPhase_CategoryFilter();
			~BroadPhase_CategoryFilter();

			virtual void update() {};
			virtual std::vector<std::pair<collidingObject*, collidingObject*>> findPairs();
			virtual std::vector<collidingObject*> findShapePairs(c2Shape* shape, uint16_t collision_mask); //todo maybe use collidingObject* instead of shape/coll_mask
			//virtual std::unordered_map<collidingObject*, collidingObject*> findPairs();

			virtual void add(tgf::collision::collidingObject& obj);
			//virtual void remove(collidingObject* obj);
			virtual void remove(CollisionEntity* obj);

		protected:
			// store function pointers to call when a collision did hit, mapped to categories
			std::map<CollisionCategory, std::vector<tgf::collision::collidingObject>> colliders_;
		};
	}
}