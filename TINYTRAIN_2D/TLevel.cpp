#include "tinyc2.h"
#include "TLevel.h"
#include "TTrain.h"
#include "TRailTrack.h"
#include "GameState_Running.h"
#include "TObstacle.h"
#include "InterpolateToPoint.h"

namespace tinytrain
{
	TLevel::TLevel()
	{
	}


	TLevel::~TLevel()
	{
	}

	void TLevel::onDraw(sf::RenderTarget * target)
	{
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

	void TLevel::onUpdate(float deltaTime)
	{
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
			auto zone = std::make_unique<TObstacle>(gs, false);
			zone->drawable_->setPosition(+30.0f, +30.0f);
			zone->updateCollisionShape();

			

			// create temporary component by constructor to use in copy constructor
			tgf::components::InterpolateToPoint c(zone->getPosition(), zone->getPosition() + sf::Vector2f(50.f, 0.0f), 2.0f, tgf::components::MovementType::TwoWay, true, false);
			c.start();
			zone->addNewComponent<tgf::components::InterpolateToPoint>(c);
			obstacles_.push_back(std::move(zone));

			// create target zone for the game to be won
			auto target_zone = std::make_unique<TObstacle>(gs, true);
			target_zone->setPosition(-30.0f, -30.0f);
			target_zone->updateCollisionShape();
			
			// change duration and line for the target_zone
			c.duration_ = 1.0f;
			c.setControlPoints(target_zone->getPosition(), target_zone->getPosition() + sf::Vector2f(-30.f, -30.0f));
			target_zone->addNewComponent<tgf::components::InterpolateToPoint>(c);
			
			// create movement component by variables
			//auto movement_comp = std::make_unique<tgf::components::InterpolateToPoint>();
			//movement_comp->setControlPoints(target_zone->getPosition(), target_zone->getPosition() + sf::Vector2f(-30.f, -30.0f));
			//movement_comp->duration_ = 1.0f;
			//movement_comp->type_ = tgf::components::MovementType::TwoWay;
			//movement_comp->repeat_ = false;
			//movement_comp->start();
			//target_zone->addComponent(std::move(movement_comp));

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