#include "TCollisionZone.h"
#include "TTrain.h"
#include "GameState_Running.h"

namespace tinytrain
{
	TCollisionZone::TCollisionZone(GameState_Running* gs, bool wintrigger, tgf::collision::CollisionCategory cat) :
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
		// todo: lock update behind a flag which has to be set everytime someone changes the transformation/shape
		if (drawable_ && drawableMoved_)
			updateCollisionShape_matchDrawable();
	}

	void TCollisionZone::onTriggerEnter(class CollisionEntity* other)
	{
		TTrain* train = dynamic_cast<TTrain*>(other);
	
		if (train && gs_)
		{
			// train hit a zone
			if (winningTrigger_)
				gs_->won(train);
			else
				gs_->lost(train);
		}
	}

	void TCollisionZone::onTriggerLeave(CollisionEntity * a_other)
	{
	}

	void TCollisionZone::setCollisionShape_AABB(c2v min, c2v max)
	{
		aabb_shape_.min = min;
		aabb_shape_.max = max;

		collisionShape_.type_ = C2_AABB;
		collisionShape_.shape_ = &aabb_shape_;

		if(debugShape_.getVertexCount() != 5)
		{
			debugShape_.resize(5);
			debugShape_[0].color = sf::Color::Red;
			debugShape_[1].color = sf::Color::Red;
			debugShape_[2].color = sf::Color::Red;
			debugShape_[3].color = sf::Color::Red;
			//debugShape_[4].color = sf::Color::Red;

			debugShape_.setPrimitiveType(sf::LineStrip);
		}

		debugShape_[0].position.x = debugShape_[3].position.x = aabb_shape_.min.x;
		debugShape_[1].position.x = debugShape_[2].position.x = aabb_shape_.max.x;
		debugShape_[0].position.y = debugShape_[1].position.y = aabb_shape_.min.y;
		debugShape_[2].position.y = debugShape_[3].position.y = aabb_shape_.max.y;
		debugShape_[4] = debugShape_[0];

		collisionUpdated = true;
	}

	void TCollisionZone::setCollisionShape_Poly(const c2Poly poly)
	{
		poly_shape_.count = poly.count;
		
		collisionShape_.type_ = C2_POLY;
		collisionShape_.shape_ = &poly_shape_;

		if (debugShape_.getVertexCount() != poly_shape_.count)
		{
			debugShape_.resize(5);
			debugShape_.setPrimitiveType(sf::LineStrip);
		}

		for (int i = 0; i < poly_shape_.count; i++)
		{
			debugShape_[i].color = sf::Color::Red;
			debugShape_[i].position = { poly_shape_.verts[i].x, poly_shape_.verts[i].y };

			poly_shape_.norms[i] = poly.norms[i];
			poly_shape_.verts[i] = poly.verts[i];
		}

		collisionUpdated = true;
	}

	void TCollisionZone::updateCollisionShape_matchDrawable()
	{
		if (drawable_ && drawable_->getPointCount() == 4)
		{
			c2Poly poly;
			poly.count = 4;
			auto tf = getTransform() * drawable_->getTransform();
			//auto tf = getTransform();
			auto temp = tf.transformPoint(drawable_->getPoint(0));
			poly.verts[0] = { temp.x, temp.y };
			temp = tf.transformPoint(drawable_->getPoint(1));
			poly.verts[1] = { temp.x, temp.y };
			temp = tf.transformPoint(drawable_->getPoint(2));
			poly.verts[2] = { temp.x, temp.y };
			temp = tf.transformPoint(drawable_->getPoint(3));
			poly.verts[3] = { temp.x, temp.y };

			setCollisionShape_Poly(poly);
		}
	}

	void TCollisionZone::setCollisionCategory(tgf::collision::CollisionCategory cat)
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
