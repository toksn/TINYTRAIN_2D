#include "TTrainCollisionManager.h"
#include "TObstacle.h"


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
		for (auto& category = colliders_.cbegin(); category != colliders_.cend(); ++category)
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
				while (upcoming_category != colliders_.cend())
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
					++upcoming_category;
				}
			}
		}
	}

	void TTrainCollisionManager::tryCollideTrainObject(TTrain* train, collidingObject& obj)
	{
		bool hit = false;

		// check train vs rect
		//c2MakePoly();
		auto collider = obj.obj->getCollisionShape();
		if (train->wagons_.size() && collider.shape_)
		{
			auto bb = train->wagons_[0].getGlobalBounds();
			c2AABB aabb;
			aabb.min.x = bb.left;
			aabb.min.y = bb.top;
			aabb.max.x = bb.left + bb.width;
			aabb.max.y = bb.top + bb.height;
			if (c2Collided(&aabb, NULL, C2_AABB, collider.shape_, NULL, collider.type_))
			{
				// train wagon AABB and obj.shape did collide
				float rotation = train->wagons_[0].getRotation();
				// test rotation for 0, 90, 180, 270, 360 degrees
				rotation = fmod(rotation, 90.0f);
				if (rotation == 0)
				{
					// dont have to test again because train AABB == train rect
					hit = true;
				}
				else
				{
					// can make proper hit test now
					c2Poly train_rect;
					auto temp = train->wagons_[0].getTransform().transformPoint(train->wagons_[0].getPoint(0));
					train_rect.verts[0] = { temp.x, temp.y };
					temp = train->wagons_[0].getTransform().transformPoint(train->wagons_[0].getPoint(1));
					train_rect.verts[1] = { temp.x, temp.y };
					temp = train->wagons_[0].getTransform().transformPoint(train->wagons_[0].getPoint(2));
					train_rect.verts[2] = { temp.x, temp.y };
					temp = train->wagons_[0].getTransform().transformPoint(train->wagons_[0].getPoint(3));
					train_rect.verts[3] = { temp.x, temp.y };
					train_rect.count = 4;
					hit = c2Collided(collider.shape_, NULL, collider.type_, &train_rect, NULL, C2_POLY);
				}
			}
			
		}
		else
		{
			// try to fake a point with an size zero AABB (probably the fastest of the c2 shapes to test against)
			c2AABB fake_point;
			fake_point.min = { train->getPosition().x, train->getPosition().y };
			fake_point.max = fake_point.min;

			hit = c2Collided(collider.shape_, NULL, collider.type_, &fake_point, NULL, C2_AABB);
		}
			
		
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

		auto collider1 = obj1.obj->getCollisionShape();
		auto collider2 = obj2.obj->getCollisionShape();
		if(collider1.shape_ && collider2.shape_)
			hit = c2Collided(collider1.shape_, NULL, collider1.type_, collider2.shape_, NULL, collider2.type_);

		//// check rect vs rect
		//hit = obj1.obj->collisionShape_->intersects(*obj2.obj->collisionShape_);
		//if (hit == false)
		//	hit = obj1.obj->collisionShape_->contains(sf::Vector2f(obj2.obj->collisionShape_->top, obj2.obj->collisionShape_->left));

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