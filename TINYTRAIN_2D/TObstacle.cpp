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
		drawable_ = std::make_unique<sf::RectangleShape>(rect);

		collisionShape_ = std::make_unique<c2Poly>();
		if (collisionShape_)
		{
			collisionShape_->count = 4;
			updateCollisionShape();
		}

		//winningTrigger_ = wintrigger;

		if (gs_ && gs_->getCollisionManager())
		{
			auto colli = gs_->getCollisionManager();
			if(winningTrigger_)
				colli->addToCollision(this, &TObstacle::onTriggerEnter, NULL, TTrainCollisionManager::CollisionCategory::OBSTACLE_WIN, 0);
			else
				colli->addToCollision(this, &TObstacle::onTriggerEnter, NULL);
		}
	}


	TObstacle::~TObstacle()
	{
		if (gs_ && gs_->getCollisionManager())
			gs_->getCollisionManager()->removeFromCollision(this);
	}

	void TObstacle::draw(sf::RenderTarget * target)
	{
		if(drawable_)
			target->draw(*drawable_);

		//if (drawCollisionShape_ && collisionShape_)
		//	target->draw(*collisionShape_);
	}

	void TObstacle::update(float deltaTime)
	{
	}

	void TObstacle::updateCollisionShape()
	{
		if (drawable_ && drawable_->getPointCount() == 4)
		{
			auto temp = drawable_->getTransform().transformPoint(drawable_->getPoint(0));
			collisionShape_->verts[0] = { temp.x, temp.y };
			temp = drawable_->getTransform().transformPoint(drawable_->getPoint(1));
			collisionShape_->verts[1] = { temp.x, temp.y };
			temp = drawable_->getTransform().transformPoint(drawable_->getPoint(2));
			collisionShape_->verts[2] = { temp.x, temp.y };
			temp = drawable_->getTransform().transformPoint(drawable_->getPoint(3));
			collisionShape_->verts[3] = { temp.x, temp.y };
		}
	}

	void TObstacle::onTriggerEnter(class Entity* other)
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
	c2Shape TObstacle::getCollisionShape()
	{
		c2Shape s;
		s.type_ = C2_POLY;
		s.shape_ = collisionShape_.get();
		return s;
	}
}
