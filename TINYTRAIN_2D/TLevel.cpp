#include "tinyc2.h"
#include "TLevel.h"
#include "TTrain.h"
#include "TRailTrack.h"
#include "GameState_Running.h"
#include "TObstacle.h"

namespace tinytrain
{
	TLevel::TLevel()
	{
	}


	TLevel::~TLevel()
	{
	}

	void TLevel::draw(sf::RenderTarget * target)
	{
		Entity::draw(target);
		if (train_)
			train_->draw(target);
		if (railtrack_)
			railtrack_->draw(target);

		for (int i = obstacles_.size() - 1; i >= 0; i--)
		{
			auto o = obstacles_[i].get();
			if (o)
				o->draw(target);
			else
				obstacles_.erase(obstacles_.begin() + i);
		}
	}

	void TLevel::update(float deltaTime)
	{
		Entity::update(deltaTime);
		if (train_)
			train_->update(deltaTime);
		if (railtrack_)
			railtrack_->update(deltaTime);

		for (int i = obstacles_.size()-1; i >= 0; i--)
		{
			auto o = obstacles_[i].get();
			if (o)
				o->update(deltaTime);
			else
				obstacles_.erase(obstacles_.begin()+i);
		}			
	}

	void TLevel::load(GameState_Running* gs, std::string file)
	{
		if (file.empty())
		{
			/**************************************************************************
			SIMPLE LEVEL CREATED BY CODE -- this is the minimum requirement for a level
			***************************************************************************/
			// create train for the player
			train_ = std::make_unique<TTrain>(gs);
			train_->play();
			
			// create a railtrack for the train
			railtrack_ = std::make_unique<TRailTrack>();

			railtrack_->append(sf::Vector2f(200.0f, 50.f));
			railtrack_->append(sf::Vector2f(200.0f, 100.f));
			railtrack_->append(sf::Vector2f(250.0f, 140.f));
			railtrack_->append(sf::Vector2f(150.0f, 180.f));
			railtrack_->append(sf::Vector2f(130.0f, 70.f));

			c2v start{ 150.0f, 180.f };
			c2v end{ 130.0f, 70.f };
			float dist = 10.0f;
			int angle_range = 20;
			angle_range *= 100;

			c2v seg = c2Sub(end, start);
			// 57.295779513 := rad to degre conversion (rad * 180.0/pi)
			float angle = atan2(seg.y, seg.x) * RAD_TO_DEG;

			for (size_t i = 0; i < 10; i++)
			{
				angle += ((rand() % angle_range) - angle_range * 0.5f)/100.0f;

				//lastPos.x += rand() % 200 - 100;
				//lastPos.y += rand() % 200 - 100;

				//lastPos.x += rand() % 30;
				//lastPos.y += rand() % 30;

				start = end;
				end.x += dist * cos(angle / RAD_TO_DEG);
				end.y += dist * sin(angle / RAD_TO_DEG);
				
				railtrack_->append(sf::Vector2f(end.x, end.y));
			}

			railtrack_->addTrain(train_.get());
			train_->initWagons(30);

			// create obstacles for the games to be lost
			obstacles_.push_back(std::make_unique<TObstacle>(gs, false));
			

			// create target zone for the game to be won
			auto target_zone = std::make_unique<TObstacle>(gs, true);
			target_zone->drawable_->setPosition(-30.0f, -30.0f);
			target_zone->updateCollisionShape();
			obstacles_.push_back(std::move(target_zone));			
			/************************************************************************/

			// TODO: passengers to pick up
		}
	}
	void TLevel::restart(GameState_Running* gs)
	{
		obstacles_.clear();
		load(gs);			//load(gs, currentLevelFile);
	}
}