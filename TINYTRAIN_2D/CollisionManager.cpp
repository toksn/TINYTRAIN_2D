#include "CollisionManager.h"
#include "CollisionEntity.h"
#include "BroadPhase_CategoryFilter.h"


namespace tgf
{
	namespace collision
	{
		CollisionManager::CollisionManager()
		{
			broadphase_ = std::make_unique<BroadPhase_CategoryFilter>();
		}
		
		CollisionManager::~CollisionManager()
		{
		}
		
		void CollisionManager::removeFromCollision(void * obj)
		{
			broadphase_->remove((CollisionEntity*)obj);
		}

		
		std::vector<CollisionEntity*> CollisionManager::tryCollideShape(c2Shape* shape, uint16_t collision_mask)
		{
			std::vector<CollisionEntity*> collidedObjects;
			auto broadphasehits = broadphase_->findShapePairs(shape, collision_mask);

			for (auto& c : broadphasehits)
			{
				auto collider2 = c->obj->getCollisionShape();
				if (collider2.shape_)
					if (c2Collided(shape->shape_, NULL, shape->type_, collider2.shape_, NULL, collider2.type_))
						collidedObjects.push_back(c->obj);
			}
				
			return collidedObjects;
		}
		
		bool CollisionManager::checkShapeForCollisions(c2Shape* shape, uint16_t collision_mask)
		{
			if (collision_mask == 0 || shape->shape_ == nullptr)
				return false;

			auto broadphasehits = broadphase_->findShapePairs(shape, collision_mask);

			for (auto& c : broadphasehits)
			{
				auto collider2 = c->obj->getCollisionShape();
				if (collider2.shape_)
					if (c2Collided(shape->shape_, NULL, shape->type_, collider2.shape_, NULL, collider2.type_))
						return true;
			}

			return false;
		}

		void CollisionManager::update()
		{
			broadphase_->update();

			resolveCurrentCollisions();

			auto pairs = broadphase_->findPairs();
			for(auto p : pairs)
				tryCollideObjects(*p.first, *p.second);
		}

		void CollisionManager::resolveCurrentCollisions()
		{
			auto colliders = broadphase_->getAllColliders();
			for (auto it1 = colliders.begin(); it1 != colliders.end(); )
			{
				collidingObject* obj1 = *it1;
				++it1;
				for (auto it2 = obj1->currentCollisions.begin(); it2 != obj1->currentCollisions.end(); )
				{
					collidingObject* obj2 = *it2;
					++it2;

					auto shape = obj1->obj->getCollisionShape();
					auto shape2 = obj2->obj->getCollisionShape();
					if (c2Collided(shape.shape_, NULL, shape.type_, shape2.shape_, NULL, shape2.type_) == false)
					{
						if(obj1->callback_leave)
							obj1->callback_leave(obj2->obj);
						if(obj2->callback_leave)
							obj2->callback_leave(obj1->obj);						

						// remove from currentCol->currentCollision
						obj2->currentCollisions.erase(obj1);

						// remove from c->obj->currentCollision
						obj1->currentCollisions.erase(obj2);
					}
				}
			}
		}
		
		void CollisionManager::tryCollideObjects(collidingObject& obj1, collidingObject& obj2)
		{
			bool hit = false;

			// find collisions that already took place
			bool o1o2 = obj1.currentCollisions.find(&obj2) != obj1.currentCollisions.end();
			bool o2o1 = obj2.currentCollisions.find(&obj1) != obj2.currentCollisions.end();

			auto collider1 = obj1.obj->getCollisionShape();
			auto collider2 = obj2.obj->getCollisionShape();
			
			if (collider1.shape_ && collider2.shape_ && o1o2 == false && o2o1 == false)
				hit = c2Collided(collider1.shape_, NULL, collider1.type_, collider2.shape_, NULL, collider2.type_);
			
			// call callbacks
			if (hit)
			{
				if (o2o1 == false)
				{
					if (obj1.callback_enter)
						obj1.callback_enter(obj2.obj);

					obj1.currentCollisions.emplace(&obj2);
				}
				if (o2o1 == false)
				{
					if (obj2.callback_enter)
						obj2.callback_enter(obj1.obj);

					obj2.currentCollisions.emplace(&obj1);
				}
			}
		}
	}
}