#include "TTrain.h"
#include "GameState_Running.h"

namespace tinytrain
{
	TTrain::TTrain(GameState_Running* gs)
	{
		color_firstwagon_ = sf::Color::Yellow;
		color_wagons_ = sf::Color::White;
		color_filledwagons_ = sf::Color::Blue;

		wagonsize_ = sf::Vector2f(20.0f, 10.0f);
		wagongap_ = 5.0f;
		speed_ = wagonsize_.x * 12.0f;
		passenger_count_ = 0;

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

	bool TTrain::hasCapacity(unsigned int space_needed)
	{
		return wagons_.size() - 1 - passengers_.size() >= space_needed;
	}

	void TTrain::pickUp(std::unique_ptr<TPassenger> passenger)
	{
		passengers_.push_back(std::move(passenger));
	}

	void TTrain::onUpdate(const float dt)
	{
		// move the wagons by speed * dt on the railtrack
		if(isRunning_)			
			distance_ += dt * speed_;

		calcAABB();

		// set color for the leading wagon
		if (passenger_count_ != passengers_.size())
		{
			passenger_count_ = passengers_.size();

			for (int i = 1; i < wagons_.size(); i++)
				wagons_[i].setFillColor(color_wagons_);

			for(int i = 1; i-1 < passengers_.size() && i < wagons_.size(); i++)
				wagons_[i].setFillColor(color_filledwagons_);
		}

		passengers_done_.clear();
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

	void TTrain::collision(Entity * other)
	{
		// TODO: handle car hits

		// TODO: handle self hits

		// handle passenger hits
		TPassenger* passenger = dynamic_cast<TPassenger*>(other);
		if (passenger)
		{
			TPassenger::PassengerState state = passenger->getState();
			if (state == TPassenger::PassengerState::WAIT_FOR_PICKUP)
			{
				pickUp(passenger->level_->removePassenger(passenger->id_));

				passenger->setState(TPassenger::PassengerState::COLLECTED);
			}
			else if (state == TPassenger::PassengerState::COLLECTED)
			{
				for (auto it = passengers_.begin(); it != passengers_.end(); ++it)
				{
					if (it->get() == passenger)
					{
						passenger->level_->points_ += passenger->points_;
						
						passengers_done_.push_back( std::move(*it) );
						passengers_.erase(it);

						return;
					}
				}
			}
			
		}
	}

	void TTrain::collisionEnd(Entity * other)
	{
	}

	void TTrain::onDraw(sf::RenderTarget * target)
	{
		// draw them wagons
		for (int i = 0; i < wagons_.size(); i++)
			target->draw(wagons_[i]);

		// draw the passengers (destination zones)
		for (int i = 0; i < passengers_.size(); i++)
			passengers_[i]->draw(target);

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
