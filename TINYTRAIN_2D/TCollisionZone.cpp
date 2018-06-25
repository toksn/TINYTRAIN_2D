#include "TCollisionZone.h"
#include "TTrain.h"
#include "GameState_Running.h"

namespace tinytrain
{
	TCollisionZone::TCollisionZone(GameState_Running* gs, bool wintrigger, TTrainCollisionManager::CollisionCategory cat) :
		winningTrigger_(wintrigger)
	{
		gs_ = gs;
		
		setCollisionCategory(cat);

		drawDebug_ = false;
	}


	TCollisionZone::~TCollisionZone()
	{
		if (gs_ && gs_->getCollisionManager())
			gs_->getCollisionManager()->removeFromCollision(this);
	}

	void TCollisionZone::onDraw(sf::RenderTarget * target)
	{
		if(drawDebug_)
			target->draw(debugShape_);
	}

	void TCollisionZone::onUpdate(float deltaTime)
	{
		
	}

	void TCollisionZone::onTriggerEnter(class Entity* other)
	{
		TTrain* train = dynamic_cast<TTrain*>(other);
				
#if 1		// godmode
		if (false)
#else
		if (train && gs_)
#endif
		{
			// train hit a zone
			if (winningTrigger_)
				gs_->won(train);
			else
				gs_->lost(train);
		}
	}

	void TCollisionZone::onTriggerLeave(Entity * a_other)
	{
	}

	void TCollisionZone::setCollisionShape(C2_TYPE type, void* shape)
	{
		collisionShape_.type_ = type;
		collisionShape_.shape_ = shape;

		if (type == C2_POLY)
		{
			c2Poly* p = (c2Poly*)shape;
			if (p->count != debugShape_.getVertexCount())
				debugShape_.resize(p->count);

			debugShape_.setPrimitiveType(sf::LineStrip);
			for (int i = 0; i < p->count; i++)
			{
				debugShape_[i].position.x = p->verts[i].x;
				debugShape_[i].position.y = p->verts[i].y;
				debugShape_[i].color = sf::Color::Red;
			}				
		}
		else if(debugShape_.getVertexCount() != 4)
		{
			debugShape_.resize(4);
			debugShape_[0].color = sf::Color::Red;
			debugShape_[1].color = sf::Color::Red;
			debugShape_[2].color = sf::Color::Red;
			debugShape_[3].color = sf::Color::Red;

			debugShape_.setPrimitiveType(sf::Quads);
		}

		c2Circle* c = nullptr;
		c2AABB* s = nullptr;
		
		switch (type)
		{
		case C2_CIRCLE:
			c = (c2Circle*)shape;
			debugShape_[0].position.x = debugShape_[3].position.x = c->p.x - c->r;
			debugShape_[1].position.x = debugShape_[2].position.x = c->p.x + c->r;
			debugShape_[0].position.y = debugShape_[1].position.y = c->p.y - c->r;
			debugShape_[2].position.y = debugShape_[3].position.y = c->p.y + c->r;
			break;
		case C2_AABB:
		case C2_CAPSULE:
			s = (c2AABB*)shape;
			debugShape_[0].position.x = debugShape_[3].position.x = s->min.x;
			debugShape_[1].position.x = debugShape_[2].position.x = s->max.x;
			debugShape_[0].position.y = debugShape_[1].position.y = s->min.y;
			debugShape_[2].position.y = debugShape_[3].position.y = s->max.y;
			break;
		/*case C2_CAPSULE:
			c2Capsule* s = (c2Capsule*)shape;
			debugShape_[0].position.x = debugShape_[3].position.x = s->min.x;
			debugShape_[1].position.x = debugShape_[2].position.x = s->max.x;
			debugShape_[0].position.y = debugShape_[1].position.y = s->min.y;
			debugShape_[2].position.y = debugShape_[3].position.y = s->max.y;
			break;*/
		}
	}

	tgf::collision::c2Shape TCollisionZone::getCollisionShape()
	{
		return collisionShape_;
	}
	void TCollisionZone::setCollisionCategory(tgf::collision::CollisionManager::CollisionCategory cat)
	{
		if (gs_ && gs_->getCollisionManager())
		{
			auto colli = gs_->getCollisionManager();
			colli->removeFromCollision(this);
			//tgf::collision::collisionCallbackFunc<TCollisionZone> no_callback = nullptr;
			//colli->addToCollision(this, &TCollisionZone::onTriggerEnter, no_callback, cat, 0);
			colli->addToCollision(this, &TCollisionZone::onTriggerEnter, &TCollisionZone::onTriggerLeave, cat, 0);
		}
			
	}
}
