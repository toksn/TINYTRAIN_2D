#include "BroadPhase_CategoryFilter.h"
#include "CollisionEntity.h"

namespace tgf
{
	namespace collision
	{

		BroadPhase_CategoryFilter::BroadPhase_CategoryFilter()
		{
		}


		BroadPhase_CategoryFilter::~BroadPhase_CategoryFilter()
		{
		}

		std::vector<std::pair<tgf::collision::collidingObject*, tgf::collision::collidingObject*>> BroadPhase_CategoryFilter::findPairs()
		{
			std::vector<std::pair<tgf::collision::collidingObject*, tgf::collision::collidingObject*>> pairs;

			// todo: into category_only_broadphase:
			// check all collider object against each other, only test for collision when they have their masks set up properly
			for (auto& category = colliders_.begin(); category != colliders_.end(); ++category)
			{
				//for (auto collider : category->second)
				for (int i = 0; i < category->second.size(); i++)
				{
					auto collider = category->second[i];
					if ((collider.collision_mask & (uint16_t)category->first) != 0)
					{
						// only check against upcoming objects FROM THE SAME CATEGORY to prevent finding collisions twice
						for (int j = i + 1; j < category->second.size(); j++)
						{
							// check if upcoming object has current category in its collision mask
							auto other = category->second[j];
							if ((other.collision_mask & (uint16_t)category->first) != 0)
								pairs.push_back(std::make_pair<collidingObject*, collidingObject*>(&collider, &other));
						}
					}

					// only check against all objects FROM UPCOMPING CATEGORIES to prevent finding collisions twice
					auto upcoming_category = category;
					++upcoming_category;
					while (upcoming_category != colliders_.end())
					{
						// check if this upcoming category is in the collision mask of the object
						if ((collider.collision_mask & (uint16_t)upcoming_category->first) != 0)
						{
							for (auto& other : upcoming_category->second)
							{
								// check if object from the upcoming category has current category in its collision mask
								if ((other.collision_mask & (uint16_t)category->first) != 0)
									pairs.push_back(std::make_pair<collidingObject*, collidingObject*>(&collider, &other));
							}
						}
						++upcoming_category;
					}
				}
			}
			return pairs;
		}
		std::vector<collidingObject*> BroadPhase_CategoryFilter::findShapePairs(c2Shape * shape, uint16_t collision_mask)
		{
			std::vector<collidingObject*> collidedEntities;
			if (collision_mask != 0/* && shape->shape_ != nullptr*/)
			{
				for (auto& category : colliders_)
				{
					if ((collision_mask & (uint16_t)category.first) != 0)
					{
						for (auto& other : category.second)
						{
							collidedEntities.push_back(&other);
						}
					}
				}
			}

			return collidedEntities;
		}
		void BroadPhase_CategoryFilter::add(collidingObject & obj)
		{
			colliders_[(CollisionCategory)obj.collision_category].push_back(obj);
		}
		void BroadPhase_CategoryFilter::remove(CollisionEntity * obj)
		{
			for (auto o = colliders_.begin(); o != colliders_.end(); ++o)
			{
				for (int i = o->second.size() - 1; i >= 0; i--)
				{
					if (o->second[i].obj == obj)
						o->second.erase(o->second.begin() + i);
				}
			}
		}
	}
}