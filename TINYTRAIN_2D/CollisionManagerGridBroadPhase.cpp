#include "CollisionManagerGridBroadPhase.h"
#include "CollisionEntity.h"
#include "MathHelper2D.h"

namespace tgf
{
	namespace collision
	{
		CollisionManagerGridBroadPhase::CollisionManagerGridBroadPhase()
		{
		}
		CollisionManagerGridBroadPhase::~CollisionManagerGridBroadPhase()
		{
		}

		void CollisionManagerGridBroadPhase::update()
		{
			updateGrid();
			/*
			std::vector<collidingObject*> already_checked;
			// check all collider object against each other, only test for collision when they have their masks set up properly
			for (auto& category = colliders_.begin(); category != colliders_.end(); ++category)
			{
				//for (auto collider : category->second)
				for (int i = 0; i < category->second.size(); i++)
				{
					auto& collider = category->second[i];
					already_checked.push_back(&collider);

					mincell.x = aabb.min.x / broadGrid_cellsize_;
					mincell.y = aabb.min.y / broadGrid_cellsize_;
					maxcell.x = aabb.max.x / broadGrid_cellsize_;
					maxcell.y = aabb.max.y / broadGrid_cellsize_;

					for (auto x = mincell.x; x <= maxcell.x; x++)
					{
						for (auto y = mincell.y; y <= maxcell.y; y++)
						{
							auto cellmembers = getCellMembers(x, y);
							for (auto m : cellmembers)
							{
								if (std::find(already_checked.begin(), already_checked.end(), m))
									continue;
								else if(collider.collision_mask & m.collision_category && m.collision_mask & collider.collision_category)
									tryCollideObjects(collider, *m);	// including mask check
							}
						}
					}
				}
			}*/
		}

		void CollisionManagerGridBroadPhase::initGridSize(sf::Vector2i gridsize, float cellsize, sf::Vector2f pos)
		{
			// todo
		}

		void CollisionManagerGridBroadPhase::updateGrid()
		{
			for (auto& c : colliders_)
			{
				for (auto& o : c.second)
				{
					if (o.obj->collisionUpdated == false)
						continue;

					o.obj->collisionUpdated = false;
					c2Shape col = o.obj->getCollisionShape();
					c2AABB aabb = tgf::math::MathHelper2D::calc_aabb(col.shape_, col.type_);

					// move by gridpos
					aabb.min.x -= broadGrid_posTopLeft_.x;
					aabb.min.y -= broadGrid_posTopLeft_.y;
					aabb.max.x -= broadGrid_posTopLeft_.x;					
					aabb.max.y -= broadGrid_posTopLeft_.y;

					sf::Vector2i mincell, maxcell;
					mincell.x = aabb.min.x / broadGrid_cellsize_;
					mincell.y = aabb.min.y / broadGrid_cellsize_;
					maxcell.x = aabb.max.x / broadGrid_cellsize_;
					maxcell.y = aabb.max.y / broadGrid_cellsize_;
					
					int minpt_index, maxpt_index;
					if (mincell.x < 0 || mincell.y < 0 || mincell.x >= broadGrid_size_.x || mincell.y >= broadGrid_size_.y)
						minpt_index = -1;
					else
						minpt_index = mincell.x + mincell.y * broadGrid_size_.x;

					if (maxcell.x < 0 || maxcell.y < 0 || maxcell.x >= broadGrid_size_.x || maxcell.y >= broadGrid_size_.y)
						maxpt_index = -1;
					else
						maxpt_index = maxcell.x + maxcell.y * broadGrid_size_.x;

					
				}
			}
		}

	}
}