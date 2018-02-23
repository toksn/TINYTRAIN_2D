#include "TObstacle.h"
#include "TTrain.h"
#include "GameState_Running.h"

namespace tinytrain
{
	TObstacle::TObstacle(GameState_Running* gs, bool wintrigger) :
		winningTrigger_(wintrigger)
	{
		gs_ = gs;
		drawCollisionShape_ = true;

		collisionShape_ = std::make_unique<sf::FloatRect>(0.0f, 0.0f, 20.0f, 20.0f);
		sf::RectangleShape rect(sf::Vector2f(collisionShape_->width, collisionShape_->height));
		rect.setFillColor(sf::Color::Red);
		drawable_ = std::make_unique<sf::RectangleShape>(rect);
		
		//winningTrigger_ = wintrigger;

		if (gs_ && gs_->getCollisionManager())
		{
			auto colli = gs_->getCollisionManager();
			if(winningTrigger_)
				colli->addToCollision(this, &TObstacle::onTriggerEnter, TTrainCollisionManager::CollisionCategory::OBSTACLE_WIN, 0);
			else
				colli->addToCollision(this, &TObstacle::onTriggerEnter);
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
}
