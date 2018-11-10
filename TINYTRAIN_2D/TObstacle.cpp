#include "TObstacle.h"
#include "TTrain.h"
#include "GameState_Running.h"

namespace tinytrain
{
	TObstacle::TObstacle(GameState_Running* gs, bool wintrigger) :
		winningTrigger_(wintrigger)
	{
		gs_ = gs;
		//drawCollisionShape_ = true;
		sf::RectangleShape rect(sf::Vector2f(20.0f, 20.0f));
		rect.setFillColor(wintrigger? sf::Color::Green : sf::Color::Red);
		
		// this caused 10px off errors continously - removed
		//rect.setOrigin(rect.getSize() * 0.5f);

		drawable_ = std::make_unique<sf::RectangleShape>(rect);

		poly_ = std::make_unique<c2Poly>();
		if (poly_)
		{
			poly_->count = 4;
			updateCollisionShape();
		}

		//winningTrigger_ = wintrigger;

		if (gs_ && gs_->getCollisionManager())
		{
			auto colli = gs_->getCollisionManager();
			
			tgf::collision::collisionCallbackFunc<TObstacle> no_callback = nullptr;
			if (winningTrigger_)
				// add to collision, only colliding with trains
				colli->addToCollision(this, &TObstacle::onTriggerEnter, no_callback, tgf::collision::CollisionCategory::STATIC_CATEGORY_2, 0);
			else
				// add to collision colliding with trains and dynamic_category_1
				colli->addToCollision(this, &TObstacle::onTriggerEnter, no_callback, tgf::collision::CollisionCategory::DYNAMIC_CATEGORY_1, 0);
		}
	}


	TObstacle::~TObstacle()
	{
		if (gs_ && gs_->getCollisionManager())
			gs_->getCollisionManager()->removeFromCollision(this);
	}

	void TObstacle::onDraw(sf::RenderTarget * target)
	{
		if(drawable_)
			target->draw(*drawable_, getTransform());

		//if (drawCollisionShape_ && poly_)
		//	target->draw(*poly_);
	}

	void TObstacle::onUpdate(float deltaTime)
	{
		// todo: lock update behind a flag which has to be set everytime someone changes the transformation/shape
		//if(collisionUpdated)
			updateCollisionShape();
	}

	void TObstacle::updateCollisionShape()
	{
		if (drawable_ && drawable_->getPointCount() == 4)
		{
			auto tf = getTransform() * drawable_->getTransform();
			//auto tf = getTransform();
			auto temp = tf.transformPoint(drawable_->getPoint(0));
			poly_->verts[0] = { temp.x, temp.y };
			temp = tf.transformPoint(drawable_->getPoint(1));
			poly_->verts[1] = { temp.x, temp.y };
			temp = tf.transformPoint(drawable_->getPoint(2));
			poly_->verts[2] = { temp.x, temp.y };
			temp = tf.transformPoint(drawable_->getPoint(3));
			poly_->verts[3] = { temp.x, temp.y };

			//collisionUpdated = true;
		}
	}

	void TObstacle::onTriggerEnter(class CollisionEntity* other)
	{
		TTrain* train = dynamic_cast<TTrain*>(other);
	
		if (train && gs_)
		{
			// train hit an obstacle
			if (winningTrigger_)
				gs_->won(train);
			else
				gs_->lost(train);
		}
	}


	tgf::collision::c2Shape TObstacle::getCollisionShape()
	{
		tgf::collision::c2Shape s;
		s.type_ = C2_POLY;
		s.shape_ = poly_.get();
		return s;
	}
}
