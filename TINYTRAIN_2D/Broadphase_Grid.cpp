#include "Broadphase_Grid.h"
#include "CollisionEntity.h"
#include "MathHelper2D.h"

namespace tgf
{
	namespace collision
	{
		Broadphase_Grid::Broadphase_Grid()
		{
		}
		Broadphase_Grid::~Broadphase_Grid()
		{
		}
		
		void Broadphase_Grid::initGridSize(unsigned short x, unsigned short y, float cellsize, sf::Vector2f pos)
		{
			// todo broadphase
		}

		void Broadphase_Grid::update()
		{
			for (auto& c : colliders_)
			{
				if (c.obj->collisionUpdated == false)
					continue;

				c.obj->collisionUpdated = false;
				c2Shape col = c.obj->getCollisionShape();
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

				// todo broadphase
				// insert into cells min to max

				// optional: save grid cells into objects (for faster update, remove)

			}
		}

		std::vector<std::pair<collidingObject*, collidingObject*>> Broadphase_Grid::findPairs()
		{
			std::vector<std::pair<collidingObject*, collidingObject*>> pairs;
			std::vector<collidingObject*> already_checked;			

			for (auto& collider : colliders_)
			{
				already_checked.push_back(&collider);

				// find other objects in the same grid cells as collider
				auto others = findPairs_forObject(&collider, already_checked);
				
				// add them to the found pairs
				for(auto other : others)
					pairs.push_back(std::make_pair(&collider, other));
			}

			return pairs;
		}

		std::vector<collidingObject*> Broadphase_Grid::findShapePairs(c2Shape * shape, uint16_t collision_mask)
		{
			// todo broadphase

			//collidingObject c;
			//CollisionEntity e;
			return std::vector<collidingObject*>();
		}

		// find other objects in the same grid cells as collider
		std::vector<collidingObject*> Broadphase_Grid::findPairs_forObject(collidingObject * collider, std::vector<collidingObject*> ignore_objects)
		{
			std::vector<collidingObject*> pairs;
			sf::Vector2i mincell, maxcell;

			c2Shape col = collider->obj->getCollisionShape();
			c2AABB aabb = tgf::math::MathHelper2D::calc_aabb(col.shape_, col.type_);

			// move by gridpos
			aabb.min.x -= broadGrid_posTopLeft_.x;
			aabb.min.y -= broadGrid_posTopLeft_.y;
			aabb.max.x -= broadGrid_posTopLeft_.x;
			aabb.max.y -= broadGrid_posTopLeft_.y;

			mincell.x = aabb.min.x / broadGrid_cellsize_;
			mincell.y = aabb.min.y / broadGrid_cellsize_;
			maxcell.x = aabb.max.x / broadGrid_cellsize_;
			maxcell.y = aabb.max.y / broadGrid_cellsize_;

			for (auto x = mincell.x; x <= maxcell.x; x++)
			{
				for (auto y = mincell.y; y <= maxcell.y; y++)
				{
					auto cellmembers = cellMembers(x, y);
					for (auto other : cellmembers)
					{
						if (other == collider)
							continue;
						if (std::find(ignore_objects.begin(), ignore_objects.end(), other) != ignore_objects.end())
							continue;

						else if (collider->collision_mask & other->collision_category && other->collision_mask & collider->collision_category)
							pairs.push_back(other);
					}
				}
			}

			return pairs;
		}

		void Broadphase_Grid::add(tgf::collision::collidingObject & obj)
		{
			colliders_.push_back(obj);
		}

		void Broadphase_Grid::remove(CollisionEntity * obj)
		{
			for (int i = 0; i < colliders_.size(); i++)
			{
				if (colliders_[i].obj == obj)
				{
					colliders_.erase(colliders_.begin() + i);
					break;
				}
			}		
		}

		std::vector<collidingObject*> Broadphase_Grid::cellMembers(unsigned short x, unsigned short y)
		{
			// todo broad
			return std::vector<collidingObject*>();
		}

	}
}