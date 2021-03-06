#include "TTrainCollisionManager.h"
#include "CollisionEntity.h"
#include "Broadphase_Grid.h"

namespace tinytrain
{

	TTrainCollisionManager::TTrainCollisionManager()
	{
		broadphase_ = std::make_unique<tgf::collision::Broadphase_Grid>();
	}


	TTrainCollisionManager::~TTrainCollisionManager()
	{
	}
	
	void TTrainCollisionManager::addTrainToCollision(TTrain* train)
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
			CollisionManager::removeFromCollision(obj);
	}

	std::vector<tgf::collision::CollisionEntity*> TTrainCollisionManager::tryCollideShape(tgf::collision::c2Shape* shape, uint16_t collisionmask)
	{
		auto rc = CollisionManager::tryCollideShape(shape, collisionmask);

		for (auto train : trains_)
		{
			c2AABB train_aabb;
			auto aabb = train->getAABB();
			train_aabb.min = { aabb.left, aabb.top };
			train_aabb.max = { aabb.left + aabb.width, aabb.top + aabb.height };
			if (c2Collided(shape->shape_, NULL, shape->type_, &train_aabb, NULL, C2_AABB))
			{
				c2Poly wagon_rect;
				for (auto& w : train->wagons_)
				{
					auto temp = w.getTransform().transformPoint(w.getPoint(0));
					wagon_rect.verts[0] = { temp.x, temp.y };
					temp = w.getTransform().transformPoint(w.getPoint(1));
					wagon_rect.verts[1] = { temp.x, temp.y };
					temp = w.getTransform().transformPoint(w.getPoint(2));
					wagon_rect.verts[2] = { temp.x, temp.y };
					temp = w.getTransform().transformPoint(w.getPoint(3));
					wagon_rect.verts[3] = { temp.x, temp.y };
					wagon_rect.count = 4;
					if (c2Collided(shape->shape_, NULL, shape->type_, &wagon_rect, NULL, C2_POLY))
					{
						rc.push_back(train);
						break;
					}
				}
			}
		}

		return rc;
	}

	bool TTrainCollisionManager::checkShapeForCollisions(tgf::collision::c2Shape* shape, uint16_t collisionmask)
	{
		for (auto& train : trains_)
		{
			c2AABB train_aabb;
			auto aabb = train->getAABB();
			train_aabb.min = { aabb.left, aabb.top };
			train_aabb.max = { aabb.left + aabb.width, aabb.top + aabb.height };
			if (c2Collided(shape->shape_, NULL, shape->type_, &train_aabb, NULL, C2_AABB))
			{
				c2Poly wagon_rect;
				for (auto& w : train->wagons_)
				{
					auto temp = w.getTransform().transformPoint(w.getPoint(0));
					wagon_rect.verts[0] = { temp.x, temp.y };
					temp = w.getTransform().transformPoint(w.getPoint(1));
					wagon_rect.verts[1] = { temp.x, temp.y };
					temp = w.getTransform().transformPoint(w.getPoint(2));
					wagon_rect.verts[2] = { temp.x, temp.y };
					temp = w.getTransform().transformPoint(w.getPoint(3));
					wagon_rect.verts[3] = { temp.x, temp.y };
					wagon_rect.count = 4;
					if (c2Collided(shape->shape_, NULL, shape->type_, &wagon_rect, NULL, C2_POLY))
						return true;
				}
			}
		}

		return CollisionManager::checkShapeForCollisions(shape, collisionmask);
	}

	void TTrainCollisionManager::resolveCurrentCollisions()
	{
		CollisionManager::resolveCurrentCollisions();

		for (auto& train : trains_)
		{
			// resolve train current collisions
			for (auto it = train->currentCollisions.begin(); it != train->currentCollisions.end();)
			{
				tgf::collision::collidingObject* obj = *it;
				if (obj == nullptr)
				{
					it = train->currentCollisions.erase(it);
					continue;
				}
				auto collider = obj->obj->getCollisionShape();
				++it;
				if (train->wagons_.size() && collider.shape_)
				{
#if 0		// TODO: re-enable when more more than just one wagons are checked
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
#else		constexpr if(true)
						{
#endif
							{
								// TODO: check for more than just the first wagon
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
								if (c2Collided(collider.shape_, NULL, collider.type_, &train_rect, NULL, C2_POLY) == false)
								{
									//if(train->collisionEnd)
									train->collisionEnd(obj->obj);

									if (obj->callback_leave)
										obj->callback_leave(train);
									
									train->currentCollisions.erase(obj);
								}
							}
						}
				}
			}
		}
	}

	void TTrainCollisionManager::update()
	{
		// check all collider object against each other, only test for collision when they have their masks set up properly
		CollisionManager::update();

		// check for collisions of trains against any obstacle based entity (this is special as trains always collide with everything and have no TObstacle base)
		for (auto& train : trains_)
		{
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

			tgf::collision::c2Shape s;
			s.shape_ = &train_rect;
			s.type_ = C2_POLY;

			auto all_colliders = broadphase_->findShapePairs(&s, 0xFFFF);

			for (auto collider : all_colliders)//.begin(); o != all_colliders.end(); ++o)
			{
				tryCollideTrainObject(train, collider);
			}
		}
	}

	void TTrainCollisionManager::tryCollideTrainObject(TTrain* train, tgf::collision::collidingObject* obj)
	{
		if (train == nullptr)
			return;

		bool hit = false;
		auto collider = obj->obj->getCollisionShape();

		// find collisions that already took place
		if (train->currentCollisions.find(obj) != train->currentCollisions.end())
			return;

		if (train->wagons_.size() && collider.shape_)
		{
#if 0		// TODO: re-enable when more more than just one wagons are checked
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
#else		constexpr if(true)
			{
#endif
				{
					// TODO: check for more than just the first wagon
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
			train->currentCollisions.emplace(obj);

			if (obj->callback_enter)
				obj->callback_enter(train);

			train->collision(obj->obj);
		}
	}
}