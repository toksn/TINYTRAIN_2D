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
			//for (auto&c : cells_)
			//	c.members.clear();

			for (auto& a : colliders_)
			{
				auto& c = a.obj;
				if (c.obj == nullptr || c.obj->collisionUpdated == false)
					continue;

				sf::Vector2i mincell, maxcell;
				c.obj->collisionUpdated = false;
				c2Shape col = c.obj->getCollisionShape();
				calcGridCells(col, mincell, maxcell);
								
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

					//printf("moving object from cell range (%i,%i)-(%i,%i) to (%i, %i)-(%i, %i)\n", a.x_min, a.y_min, a.x_max, a.y_max, mincell.x, mincell.y, maxcell.x, maxcell.y);
					// save grid cells into objects (for faster update, remove)
					a.x_max = maxcell.x;
					a.x_min = mincell.x;
					a.y_max = maxcell.y;
					a.y_min = mincell.y;
				

				
			}
		
			//debug_checkGridAndCells();
		}

		void Broadphase_Grid::debug_checkGridAndCells()
		{
			for (auto& a : colliders_)
			{
				if (a.obj.obj == nullptr)
					continue;

				auto shape = a.obj.obj->getCollisionShape();
				sf::Vector2i mincell, maxcell;
				calcGridCells(shape, mincell, maxcell);

				if (a.x_min != mincell.x || a.y_min != mincell.y)
					printf("error: cell doesnt match\n");

				if (a.x_max != maxcell.x || a.y_max != maxcell.y)
					printf("error: cell doesnt match\n");
			}

			for (int i = 0; i < cells_.size(); i++)
			{
				auto &c = cells_[i];
				auto cell_index = tgf::math::MathHelper2D::getArrayCoordsFromIndex(i, gridsize_.x);
				for (auto obj : c.members)
				{
					if (obj->obj == nullptr)
						continue;
					auto shape = obj->obj->getCollisionShape();
					sf::Vector2i mincell, maxcell;
					calcGridCells(shape, mincell, maxcell);
					
					if (cell_index.first < mincell.x || cell_index.first > maxcell.x || cell_index.second < mincell.y || cell_index.second > maxcell.y)
						printf("object in cell (%i,%i) doesnt match the object cell range (%i,%i)-(%i,%i)\n", cell_index.first, cell_index.second, mincell.x, mincell.y, maxcell.x, maxcell.y);
				}
			}
		}

		std::vector<std::pair<collidingObject*, collidingObject*>> Broadphase_Grid::findPairs()
		{
			std::vector<std::pair<collidingObject*, collidingObject*>> pairs;
			std::unordered_set<collidingObject*> already_checked;
			std::unordered_set<collidingObject*> others;

			for (auto& c : colliders_)
			{
				auto& collider = c.obj;

				// find other objects in the same grid cells as collider
				findPairs_forObject(&collider, already_checked, others);
				
				// add them to the found pairs
				for(auto other : others)
					pairs.push_back(std::make_pair(&collider, other));
			}

			return pairs;
		}

		std::unordered_set<collidingObject*> Broadphase_Grid::getAllColliders()
		{
			std::unordered_set<collidingObject*> col;
			col.reserve(colliders_.size());
			for (auto& o : colliders_)
				col.emplace(&o.obj);
			return col;
		}

		std::unordered_set<collidingObject*> Broadphase_Grid::findShapePairs(c2Shape * shape, uint16_t collision_mask)
		{
			std::unordered_set<collidingObject*> pairs;
			sf::Vector2i mincell, maxcell;
			calcGridCells(*shape, mincell, maxcell);
			
			for (auto x = mincell.x; x <= maxcell.x; x++)
			{
				for (auto y = mincell.y; y <= maxcell.y; y++)
				{
					auto& cellmembers = cells_[x + y * gridsize_.x].members;	// cellMembers(x, y);
					for (auto other : cellmembers)
					{
						if (collision_mask & other->collision_category)
							pairs.emplace(other);
					}
				}
			}

			return pairs;
		}

		void Broadphase_Grid::calcGridCells(tgf::collision::c2Shape shape, sf::Vector2i& mincell, sf::Vector2i& maxcell)
		{
			c2AABB aabb = tgf::math::MathHelper2D::calc_aabb(shape.shape_, shape.type_);

			// move by gridpos
			aabb.min.x -= posTopLeft_.x;
			aabb.min.y -= posTopLeft_.y;
			aabb.max.x -= posTopLeft_.x;
			aabb.max.y -= posTopLeft_.y;

			mincell.x = c2Min(c2Max(aabb.min.x / cellsize_, 0.0f), gridsize_.x - 1);
			mincell.y = c2Min(c2Max(aabb.min.y / cellsize_, 0.0f), gridsize_.y - 1);
			maxcell.x = c2Min(c2Max(aabb.max.x / cellsize_, 0.0f), gridsize_.x - 1);
			maxcell.y = c2Min(c2Max(aabb.max.y / cellsize_, 0.0f), gridsize_.y - 1);
		}

		// find other objects in the same grid cells as collider
		void Broadphase_Grid::findPairs_forObject(collidingObject * collider, std::unordered_set<collidingObject*>& ignore_objects, std::unordered_set<collidingObject*>& pairs)
		{
			sf::Vector2i mincell, maxcell;
			pairs.clear();

			if (collider->obj == nullptr)
				return;

			c2Shape col = collider->obj->getCollisionShape();
			calcGridCells(col, mincell, maxcell);

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
							
							pairs.emplace(other);
						}
					}
				}
			}

			// only ignore this object further down the line when it occupies more than 1 cell
			if (mincell != maxcell)
				ignore_objects.insert(collider);

			return;
		}

		void Broadphase_Grid::add(tgf::collision::collidingObject & obj)
		{
			object_info info;
			info.obj = obj;
			colliders_.push_back(info);
		}

		void Broadphase_Grid::remove(CollisionEntity * obj)
		{
			printf("broadphase remove obj called...\n");
			for (auto c = colliders_.begin(); c != colliders_.end(); ++c)
			{
				if (c->obj.obj == obj)
				{
					// remove from currentCollisions in other objects
					for(auto obj2 : c->obj.currentCollisions)
						obj2->currentCollisions.erase(&c->obj);

					//printf("found obj... removing from cells (%i,%i)-(%i,%i)\n", c->x_min, c->y_min, c->x_max, c->y_max);
					// remove collidingobject from its cells
					removeCollidingObjectFromCells(*c);

					//colliders_[i].obj.obj = nullptr;
					colliders_.erase(c);
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
			//printf("removing object from cell. (%i,%i) to (%i,%i), erasing from:\t", obj.x_min, obj.y_min, obj.x_max, obj.y_max);
			// remove collidingobject from its cells
			for (int x = obj.x_min; x <= obj.x_max; x++)
			{
				for (int y = obj.y_min; y <= obj.y_max; y++)
				{
					auto& cellmembers = cells_[x + y * gridsize_.x].members;
					int size = cellmembers.size();
					for (auto member = cellmembers.begin(); member != cellmembers.end(); ++member)
					{
						if (*member == &obj.obj)
						{
							//printf("(%i, %i)\t", x, y);
							cellmembers.erase(member);
							//cellmembers[t] = cellmembers[cellmembers.size() - 1];
							//cellmembers.pop_back();

							break;
						}
					}
					if(size==cellmembers.size())
						printf("error: tried to remove an object from broadphase grid cell. didnt find it!\n");
				}
			}
			//printf("\n");
		}

	}
}