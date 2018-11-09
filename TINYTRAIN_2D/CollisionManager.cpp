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

			auto pairs = broadphase_->findPairs();
			for(auto p : pairs)
				tryCollideObjects(*p.first, *p.second);
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
						obj1.callback_enter(obj2.obj);

					obj1.currentCollisions.push_back(obj2.obj);
				}

				if (o2o1 == obj2.currentCollisions.end())
				{
					if (obj2.callback_enter)
						obj2.callback_enter(obj1.obj);

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
						obj1.callback_leave(obj2.obj);
				}


				if (o2o1 != obj2.currentCollisions.end())
				{
					obj2.currentCollisions.erase(o2o1);

					// callback on collision end
					if (obj2.callback_leave)
						obj2.callback_leave(obj1.obj);
				}
			}
		}
	}
}