#include "TObstacle.h"
#include "TTrain.h"
#include "GameState_Running.h"

namespace tinytrain
{
	TObstacle::TObstacle(GameState_Running* gs)
	{
		gs_ = gs;
		drawCollisionShape_ = true;
		winningTrigger_ = true;

		if (gs_ && gs_->getCollisionManager())
		{
			auto colli = gs_->getCollisionManager();
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

		if (drawCollisionShape_ && collisionShape_)
			target->draw(*collisionShape_);
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
