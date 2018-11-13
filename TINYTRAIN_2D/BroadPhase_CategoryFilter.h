#pragma once
#include <functional>
#include <vector>
#include <map>
#include "BroadPhase.h"
namespace tgf
{
	namespace collision
	{
		class BroadPhase_CategoryFilter : public Broadphase
		{
		public:
			BroadPhase_CategoryFilter();
			~BroadPhase_CategoryFilter();

			// Inherited via Broadphase
			virtual void update() override;
			virtual std::vector<std::pair<collidingObject*, collidingObject*>> findPairs() override;
			virtual std::unordered_set<collidingObject*> findShapePairs(c2Shape * shape, uint16_t collision_mask) override;
			virtual std::unordered_set<collidingObject*> getAllColliders() override;
			virtual void add(tgf::collision::collidingObject & obj) override;
			virtual void remove(CollisionEntity * obj) override;

		protected:
			// store function pointers to call when a collision did hit, mapped to categories
			std::map<CollisionCategory, std::vector<tgf::collision::collidingObject>> colliders_;
		};
	}
}