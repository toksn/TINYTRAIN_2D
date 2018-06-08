#include "CollisionManager.h"
#include "CollisionEntity.h"


namespace tgf
{
	namespace collision
	{
		CollisionManager::CollisionManager()
		{
		}


		CollisionManager::~CollisionManager()
		{
		}
		
		void CollisionManager::removeFromCollision(void * obj)
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

		std::vector<Entity*> CollisionManager::tryCollideShape(c2Shape shape, short collision_mask)
		{
			std::vector<Entity*> collidedEntities;

			if (collision_mask != 0 && shape.shape_ != nullptr)
			{
				for (auto& category : colliders_)
				{
					if ((collision_mask & (short)category.first) != 0)
					{
						for (auto& other : category.second)
						{
							auto collider2 = other.obj->getCollisionShape();
							if (collider2.shape_)
							{
								if (c2Collided(shape.shape_, NULL, shape.type_, collider2.shape_, NULL, collider2.type_))
									collidedEntities.push_back(other.obj);
							}
						}
					}
				}
			}

			return collidedEntities;
		}

		bool CollisionManager::checkShapeForCollisions(c2Shape shape, short collision_mask)
		{
			if (collision_mask == 0 || shape.shape_ == nullptr)
				return false;

			for (auto& category : colliders_)
			{ 
				if ((collision_mask & (short)category.first) != 0)
				{
					for (auto& other : category.second)
					{
						auto collider2 = other.obj->getCollisionShape();
						if (collider2.shape_ && collider2.shape_ != shape.shape_)
						{
							if (c2Collided(shape.shape_, NULL, shape.type_, collider2.shape_, NULL, collider2.type_))
								return true;
						}
					}
				}
			}

			return false;
		}

		void CollisionManager::update()
		{
			// check all collider object against each other, only test for collision when they have their masks set up properly
			for (auto& category = colliders_.begin(); category != colliders_.end(); ++category)
			{
				//for (auto collider : category->second)
				for (int i = 0; i < category->second.size(); i++)
				{
					auto collider = category->second[i];
					if((collider.collision_mask & (short)category->first) != 0)
					{
						// only check against upcoming objects from the same category to prevent finding collisions twice
						for (int j = i + 1; j < category->second.size(); j++)
						{
							// check if upcoming object has current category in its collision mask
							auto other = category->second[j];
							if((other.collision_mask & (short)category->first) != 0)
								tryCollideObjects(collider, other);
						}
					}

					// only check against all objects of the upcoming categories to prevent finding collisions twice
					auto upcoming_category = category;
					++upcoming_category;
					while (upcoming_category != colliders_.end())
					{
						// check if this upcoming category is in the collision mask of the object
						if((collider.collision_mask & (short)upcoming_category->first) != 0)
						{
							for (auto& other : upcoming_category->second)
							{
								// check if object from the upcoming category has current category in its collision mask
								if((other.collision_mask & (short)category->first) != 0)
									tryCollideObjects(collider, other);
							}
						}
						++upcoming_category;
					}
				}
			}
		}

		void CollisionManager::tryCollideObjects(collidingObject& obj1, collidingObject& obj2)
		{
			bool hit = false;

			auto collider1 = obj1.obj->getCollisionShape();
			auto collider2 = obj2.obj->getCollisionShape();
			if (collider1.shape_ && collider2.shape_)
				hit = c2Collided(collider1.shape_, NULL, collider1.type_, collider2.shape_, NULL, collider2.type_);

			// find collisions that already took place
			auto o1o2 = std::find(obj1.currentCollisions.begin(), obj1.currentCollisions.end(), obj2.obj);
			auto o2o1 = std::find(obj2.currentCollisions.begin(), obj2.currentCollisions.end(), obj1.obj);

			// call callbacks
			if (hit)
			{
				if (o1o2 == obj1.currentCollisions.end())
				{
					if (obj1.callback_enter)
						obj1.callback_enter((tgf::Entity*)obj2.obj);

					obj1.currentCollisions.push_back(obj2.obj);
				}

				if (o2o1 == obj2.currentCollisions.end())
				{
					if (obj2.callback_enter)
						obj2.callback_enter((tgf::Entity*)obj1.obj);

					obj2.currentCollisions.push_back(obj1.obj);
				}
			}
			else
			{
				// remove from collision list
				if (o1o2 != obj1.currentCollisions.end())
				{
					obj1.currentCollisions.erase(o1o2);

					// callback on collision end
					if (obj1.callback_leave)
						obj1.callback_leave((tgf::Entity*)obj2.obj);
				}


				if (o2o1 != obj2.currentCollisions.end())
				{
					obj2.currentCollisions.erase(o2o1);

					// callback on collision end
					if (obj2.callback_leave)
						obj2.callback_leave((tgf::Entity*)obj1.obj);
				}
			}
		}
	}
}