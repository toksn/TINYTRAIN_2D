#include "Broadphase_Grid.h"
#include "CollisionEntity.h"
#include "MathHelper2D.h"

namespace tgf
{
	namespace collision
	{
		Broadphase_Grid::Broadphase_Grid()
		{
			printf("broadphase grid ctor ctor\n");
			// todo broadphase
			initGridSize(20, 20, 128.0f);
		}
		Broadphase_Grid::~Broadphase_Grid()
		{
		}
		
		void Broadphase_Grid::initGridSize(unsigned short x, unsigned short y, float cellsize, sf::Vector2f pos)
		{
			gridsize_.x = x;
			gridsize_.y = y;
			cellsize_ = cellsize;
			posTopLeft_ = pos;
			cells_.resize(y*x);
		}

		void Broadphase_Grid::update()
		{
			for (auto& a : colliders_)
			{
				auto& c = a.obj;
				if (c.obj->collisionUpdated == false)
					continue;

				c.obj->collisionUpdated = false;
				c2Shape col = c.obj->getCollisionShape();
				c2AABB aabb = tgf::math::MathHelper2D::calc_aabb(col.shape_, col.type_);

				// move by gridpos
				aabb.min.x -= posTopLeft_.x;
				aabb.min.y -= posTopLeft_.y;
				aabb.max.x -= posTopLeft_.x;
				aabb.max.y -= posTopLeft_.y;

				sf::Vector2i mincell, maxcell;
				mincell.x = c2Min(c2Max(aabb.min.x / cellsize_, 0.0f), gridsize_.x-1);
				mincell.y = c2Min(c2Max(aabb.min.y / cellsize_, 0.0f), gridsize_.y-1);
				maxcell.x = c2Min(c2Max(aabb.max.x / cellsize_, 0.0f), gridsize_.x-1);
				maxcell.y = c2Min(c2Max(aabb.max.y / cellsize_, 0.0f), gridsize_.y-1);

				
				/*
				int minpt_index, maxpt_index;
				if (mincell.x < 0 || mincell.y < 0 || mincell.x >= gridsize_.x || mincell.y >= gridsize_.y)
					minpt_index = -1;
				else
					minpt_index = mincell.x + mincell.y * gridsize_.x;

				if (maxcell.x < 0 || maxcell.y < 0 || maxcell.x >= gridsize_.x || maxcell.y >= gridsize_.y)
					maxpt_index = -1;
				else
					maxpt_index = maxcell.x + maxcell.y * gridsize_.x;*/

				// todo broadphase
				// insert into cells min to max
				
					removeCollidingObjectFromCells(a);

					for (int x = mincell.x; x <= maxcell.x; x++)
						for (int y = mincell.y; y <= maxcell.y; y++)
							cells_[x + y * gridsize_.x].members.push_back(&c);

					// save grid cells into objects (for faster update, remove)
					a.x_max = maxcell.x;
					a.x_min = mincell.x;
					a.y_max = maxcell.y;
					a.y_min = mincell.y;
				

				
			}
		}

		std::vector<std::pair<collidingObject*, collidingObject*>> Broadphase_Grid::findPairs()
		{
			std::vector<std::pair<collidingObject*, collidingObject*>> pairs;
			std::unordered_set<collidingObject*> already_checked;

			for (auto& c : colliders_)
			{
				auto& collider = c.obj;

				// find other objects in the same grid cells as collider
				auto others = findPairs_forObject(&collider, already_checked);
				
				// add them to the found pairs
				for(auto other : others)
					pairs.push_back(std::make_pair(&collider, other));
			}

			return pairs;
		}

		std::vector<collidingObject*> Broadphase_Grid::getAllColliders()
		{
			std::vector<collidingObject*> col;
			col.reserve(colliders_.size());
			for (auto& o : colliders_)
				col.push_back(&o.obj);
			return col;
		}

		std::vector<collidingObject*> Broadphase_Grid::findShapePairs(c2Shape * shape, uint16_t collision_mask)
		{
			std::vector<collidingObject*> pairs;
			sf::Vector2i mincell, maxcell;

			c2AABB aabb = tgf::math::MathHelper2D::calc_aabb(shape->shape_, shape->type_);

			// move by gridpos
			aabb.min.x -= posTopLeft_.x;
			aabb.min.y -= posTopLeft_.y;
			aabb.max.x -= posTopLeft_.x;
			aabb.max.y -= posTopLeft_.y;

			mincell.x = c2Min(c2Max(aabb.min.x / cellsize_, 0.0f), gridsize_.x - 1);
			mincell.y = c2Min(c2Max(aabb.min.y / cellsize_, 0.0f), gridsize_.y - 1);
			maxcell.x = c2Min(c2Max(aabb.max.x / cellsize_, 0.0f), gridsize_.x - 1);
			maxcell.y = c2Min(c2Max(aabb.max.y / cellsize_, 0.0f), gridsize_.y - 1);

			for (auto x = mincell.x; x <= maxcell.x; x++)
			{
				for (auto y = mincell.y; y <= maxcell.y; y++)
				{
					auto& cellmembers = cells_[x + y * gridsize_.x].members;	// cellMembers(x, y);
					for (auto other : cellmembers)
					{
						if (collision_mask & other->collision_category)
							pairs.push_back(other);
					}
				}
			}

			return pairs;
		}

		// find other objects in the same grid cells as collider
		std::vector<collidingObject*> Broadphase_Grid::findPairs_forObject(collidingObject * collider, std::unordered_set<collidingObject*>& ignore_objects)
		{
			std::vector<collidingObject*> pairs;
			sf::Vector2i mincell, maxcell;

			c2Shape col = collider->obj->getCollisionShape();
			c2AABB aabb = tgf::math::MathHelper2D::calc_aabb(col.shape_, col.type_);

			// move by gridpos
			aabb.min.x -= posTopLeft_.x;
			aabb.min.y -= posTopLeft_.y;
			aabb.max.x -= posTopLeft_.x;
			aabb.max.y -= posTopLeft_.y;

			mincell.x = c2Min(c2Max(aabb.min.x / cellsize_, 0.0f), gridsize_.x-1);
			mincell.y = c2Min(c2Max(aabb.min.y / cellsize_, 0.0f), gridsize_.y-1);
			maxcell.x = c2Min(c2Max(aabb.max.x / cellsize_, 0.0f), gridsize_.x-1);
			maxcell.y = c2Min(c2Max(aabb.max.y / cellsize_, 0.0f), gridsize_.y-1);

			for (auto x = mincell.x; x <= maxcell.x; x++)
			{
				for (auto y = mincell.y; y <= maxcell.y; y++)
				{
					auto& cellmembers = cells_[x + y * gridsize_.x].members;	// cellMembers(x, y);
					for (auto other : cellmembers)
					{
						// collision masks fit each other and we check another object
						if (other != collider && collider->collision_mask & other->collision_category && other->collision_mask & collider->collision_category)
						{
							
							// check for the found object to be ignored
							if (ignore_objects.find(other) != ignore_objects.end())
								continue;
							
							pairs.push_back(other);
						}
					}
				}
			}

			// only ignore this object further down the line when it occupies more than 1 cell
			if (mincell == maxcell)
				ignore_objects.insert(collider);

			return pairs;
		}

		void Broadphase_Grid::add(tgf::collision::collidingObject & obj)
		{
			object_info info;
			info.obj = obj;
			colliders_.push_back(info);
		}

		void Broadphase_Grid::remove(CollisionEntity * obj)
		{
			for (int i = 0; i < colliders_.size(); i++)
			{
				if (colliders_[i].obj.obj == obj)
				{
					// remove collidingobject from its cells
					removeCollidingObjectFromCells(colliders_[i]);

					colliders_.erase(colliders_.begin() + i);
					break;
				}
			}
		}

		std::vector<collidingObject*>& Broadphase_Grid::cellMembers(unsigned short x, unsigned short y)
		{
			return cells_[x + y * gridsize_.x].members;
		}

		void Broadphase_Grid::removeCollidingObjectFromCells(object_info& obj)
		{
			// remove collidingobject from its cells
			for (int x = obj.x_min; x <= obj.x_max; x++)
			{
				for (int y = obj.y_min; y <= obj.y_max; y++)
				{
					auto& cellmembers = cells_[x + y * gridsize_.x].members;
					for (int t = 0; t < cellmembers.size(); t++)
					{
						auto object = cellmembers[t];

						if (object == &obj.obj)
						{
							cellmembers[t] = cellmembers[cellmembers.size() - 1];
							cellmembers.pop_back();

							break;
						}
					}
				}
			}
		}

	}
}