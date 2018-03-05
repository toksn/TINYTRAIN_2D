#include "TTrain.h"
#include "GameState_Running.h"

namespace tinytrain
{
	TTrain::TTrain(GameState_Running* gs)
	{
		color_firstwagon_ = sf::Color::Yellow;
		color_wagons_ = sf::Color::White;

		wagonsize_ = sf::Vector2f(20.0f, 10.0f);
		wagongap_ = 5.0f;
		speed_ = wagonsize_.x * 5.5f;

		gs_ = gs;
		if (gs_ && gs_->getCollisionManager())
			gs_->getCollisionManager()->addTrainToCollision(this);
	}

	TTrain::~TTrain()
	{
		if (gs_ && gs_->getCollisionManager())
			gs_->getCollisionManager()->removeFromCollision(this);
	}

	void TTrain::initWagons(const unsigned int a_numberOfWagons)
	{
		if (a_numberOfWagons < wagons_.size())
		{
			while (a_numberOfWagons < wagons_.size())
				wagons_.pop_back();
		}
		else
		{

			while (a_numberOfWagons > wagons_.size())
			{
				auto rect = sf::RectangleShape(wagonsize_);
				rect.setFillColor(color_wagons_);
				rect.setOrigin(wagonsize_ / 2.0f);
				wagons_.push_back(rect);
			}
		}

		// set color for the leading wagon
		if (wagons_.size() > 0)
			wagons_[0].setFillColor(color_firstwagon_);
	}

	void TTrain::update(const float dt)
	{
		Entity::update(dt);

		// move the wagons by speed * dt on the railtrack
		if(isRunning_)			
			distance_ += dt * speed_;
	}

	sf::Vector2f TTrain::getPosition()
	{
		if (wagons_.size())
			return wagons_[0].getPosition();

		return sf::Vector2f();
	}

	void TTrain::play()
	{
		isRunning_ = true;
	}

	void TTrain::pause()
	{
		isRunning_ = false;
	}

	void TTrain::reset()
	{
		distance_ = 0.0f;
	}

	void TTrain::stop()
	{
		pause();
		reset();
	}

	void TTrain::draw(sf::RenderTarget * target)
	{
		Entity::draw(target);

		// draw them wagons
		for (int i = 0; i < wagons_.size(); i++)
			target->draw(wagons_[i]);
	}
}
