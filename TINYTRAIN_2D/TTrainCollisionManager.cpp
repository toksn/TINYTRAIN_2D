#include "TTrainCollisionManager.h"


namespace tinytrain
{

	TTrainCollisionManager::TTrainCollisionManager()
	{
	}


	TTrainCollisionManager::~TTrainCollisionManager()
	{
	}

	void TTrainCollisionManager::addToCollision(TObstacle* const object, void(TObstacle::* const mf)(tgf::Entity *), CollisionCategory category, short collisionmask)
	{
		collidingObject col;
		col.callback = std::bind(mf, object, std::placeholders::_1);
		col.obj = object;
		col.collision_mask = collisionmask;
		colliders_[category].push_back(col);
	}

	void TTrainCollisionManager::addToCollision(TTrain* train)
	{
		trains_.push_back(train);
	}

	void TTrainCollisionManager::removeFromCollision(void * obj)
	{
		bool found = false;
		for (int i = trains_.size() - 1; i >= 0; i--)
		{
			if (trains_[i] == obj)
			{
				trains_.erase(trains_.begin() + i);
				found = true;
			}
		}

		if (found == false)
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

	//void TTrainCollisionManager::draw(sf::RenderTarget * target)
	//{
	//	// maybe debug drawings of the collisions
	//}

	void TTrainCollisionManager::update()
	{
		// check for collisions of trains against any obstacle based entity (this is special as trains always collide with everything and have no TObstacle base)
		for (auto train : trains_)
		{
			for (auto o = colliders_.begin(); o != colliders_.end(); ++o)
			{
				for (auto collider : o->second)
				{
					tryCollideTrainObject(train, collider);
				}
			}
		}
		
		// check all collider object against each other, only test for collision when they have their masks set up properly
		for (auto category = colliders_.begin(); category != colliders_.end(); ++category)
		{
			//for (auto collider : category->second)
			for (int i = 0; i < category->second.size(); i++)
			{
				auto collider = category->second[i];
				if (collider.collision_mask & (short)category->first != 0)
				{
					// only check against upcoming objects from the same category to prevent finding collisions twice
					for (int j = i + 1; j < category->second.size(); j++)
					{
						// check if upcoming object has current category in its collision mask
						auto other = category->second[j];
						if (other.collision_mask & (short)category->first != 0)
							tryCollideObjects(collider, other);
					}
				}
				
				// only check against all objects of the upcoming categories to prevent finding collisions twice
				auto upcoming_category = category;
				++upcoming_category;
				while (upcoming_category != colliders_.end());
				{
					// check if this upcoming category is in the collision mask of the object
					if (collider.collision_mask & (short)upcoming_category->first != 0)
					{
						for (auto other : upcoming_category->second)
						{
							// check if object from the upcoming category has current category in its collision mask
							if (other.collision_mask & (short)category->first != 0)
								tryCollideObjects(collider, other);
						}
					}
				} 
			}
		}
	}

	void TTrainCollisionManager::tryCollideTrainObject(TTrain* train, collidingObject& obj)
	{
		bool hit = false;

		// check train vs rect
		hit = obj.obj->collisionShape_->contains(train->getPosition());
		
		// call callbacks
		if (hit)
		{
			if (obj.callback)
				obj.callback((tgf::Entity*)train);
		}
	}

	void TTrainCollisionManager::tryCollideObjects(collidingObject& obj1, collidingObject& obj2)
	{
		bool hit = false;

		// check rect vs rect
		hit = obj1.obj->collisionShape_->intersects(*obj2.obj->collisionShape_);
		if (hit == false)
			hit = obj1.obj->collisionShape_->contains(sf::Vector2f(obj2.obj->collisionShape_->top, obj2.obj->collisionShape_->left));

		// call callbacks
		if(hit)
		{
			if(obj1.callback)
				obj1.callback((tgf::Entity*)obj2.obj);
			if(obj2.callback)
				obj2.callback((tgf::Entity*)obj1.obj);
		}
	}
}