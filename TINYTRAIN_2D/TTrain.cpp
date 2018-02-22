#include "TTrain.h"

namespace tinytrain
{
	TTrain::TTrain()
	{
		color__firstwagon = sf::Color::Yellow;
		color__wagons = sf::Color::White;

		wagonsize_ = sf::Vector2f(20.0f, 10.0f);
		wagongap_ = 5.0f;
		speed_ = wagonsize_.x * 5.5f;
	}

	TTrain::~TTrain()
	{
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
				rect.setFillColor(color__wagons);
				rect.setOrigin(wagonsize_ / 2.0f);
				wagons_.push_back(rect);
			}
		}

		// set color for the leading wagon
		if (wagons_.size() > 0)
			wagons_[0].setFillColor(color__firstwagon);
	}

	void TTrain::update(const float dt)
	{
		// move the wagons by speed * dt on the railtrack
		distance_ += dt * speed_;
	}

	sf::Vector2f TTrain::getPosition()
	{
		if (wagons_.size())
			return wagons_[0].getPosition();

		return sf::Vector2f();
	}

	void TTrain::draw(sf::RenderTarget * target)
	{
		// draw them wagons
		for (int i = 0; i < wagons_.size(); i++)
			target->draw(wagons_[i]);
	}
}
