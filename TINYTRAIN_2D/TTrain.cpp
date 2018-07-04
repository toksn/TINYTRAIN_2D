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
		speed_ = wagonsize_.x * 2.5f;

		aabb_.resize(5);
		aabb_.setPrimitiveType(sf::PrimitiveType::LineStrip);

		gs_ = gs;
		if (gs_ && gs_->getCollisionManager())
			gs_->getCollisionManager()->addTrainToCollision(this);

		debugDraw_ = false;
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

	void TTrain::onUpdate(const float dt)
	{
		// move the wagons by speed * dt on the railtrack
		if(isRunning_)			
			distance_ += dt * speed_;

		calcAABB();
	}

	sf::Vector2f TTrain::getPosition()
	{
		if (wagons_.size() == 0)
			return sf::Vector2f();
			
		return wagons_[0].getPosition();		
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

	sf::FloatRect TTrain::getAABB()
	{
		if(aabb_.getVertexCount() < 4)
			return sf::FloatRect(0.0f, 0.0f, 0.0f, 0.0f);

		return sf::FloatRect(aabb_[0].position.x, aabb_[0].position.y, aabb_[2].position.x - aabb_[0].position.x, aabb_[2].position.y - aabb_[0].position.y);
	}

	void TTrain::onDraw(sf::RenderTarget * target)
	{
		// draw them wagons
		for (int i = 0; i < wagons_.size(); i++)
			target->draw(wagons_[i]);

		if (debugDraw_)
			target->draw(aabb_);
	}

	void TTrain::calcAABB()
	{
		if (wagons_.size() == 0)
		{
			auto temp = sf::Vector2f(0.0f, 0.0f);
			aabb_[0].position = temp;
			aabb_[1].position = temp;
			aabb_[2].position = temp;
			aabb_[3].position = temp;
			aabb_[4].position = temp;
		}
			

		sf::Vector2f min(FLT_MAX, FLT_MAX);
		sf::Vector2f max(FLT_MIN, FLT_MIN);
		
		for (auto& w : wagons_)
		{
			auto r = w.getGlobalBounds();
			min.x = min.x <= r.left ? min.x : r.left;
			min.y = min.y <= r.top ? min.y : r.top;

			max.x = max.x >= r.left + r.width ? max.x : r.left + r.width;
			max.y = max.y >= r.top + r.height ? max.y : r.top + r.height;
		}

		aabb_[0].position = min;
		aabb_[4].position = min;
		aabb_[2].position = max;
		
		aabb_[1].position.x = max.x;
		aabb_[1].position.y = min.y;

		aabb_[3].position.x = min.x;
		aabb_[3].position.y = max.y;
	}
}
