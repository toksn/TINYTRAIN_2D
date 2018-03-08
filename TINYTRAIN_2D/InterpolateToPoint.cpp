#include "Component.h"
#include "InterpolateToPoint.h"

namespace tgf
{
	namespace components
	{
		InterpolateToPoint::InterpolateToPoint()
		{
			// default values
			MovementType type_ = MovementType::OneWay;
			bool repeat_ = false;
			// set to negative number to fully pause before repeat
			float delayOnRepeat_ = 0.0f;
			float duration_ = 1.0f;

			// delete the component when end is reached
			bool removeOnEnd_ = true;
			float time_ = 0.0f;

			a = { 0.0f, 0.0f };
			b = { 0.0f, 0.0f };
		}

		// constructor for interpolation of an entity with default values to use for fire and forget
		InterpolateToPoint::InterpolateToPoint(sf::Vector2f start, sf::Vector2f end, float duration, MovementType type, bool repeat, bool removeOnEnd)
		{
			setControlPoints(start, end);
			duration_ = duration;
			type_ = type;
			repeat_ = repeat;
			removeOnEnd_ = removeOnEnd;
		}

		InterpolateToPoint::~InterpolateToPoint()
		{
		}
		void InterpolateToPoint::draw(sf::RenderTarget * target)
		{
			// TODO: debug draw line from A to B
		}

		void InterpolateToPoint::update(float deltaTime)
		{
			if (running_)
			{
				time_ += deltaTime;
				c2v pos;

				// todo: use delayonrepeat
				if (time_ >= duration_)
				{
					if (repeat_ || (back_ == false && type_ == MovementType::TwoWay))
					{
						// repeat in the other direction
						if(type_ == MovementType::TwoWay)
							back_ = !back_;

						time_ = time_ - duration_;
					}
					else
					{
						// stop playback and set endpoint position
						if (back_)
							owner_->setPosition(a.x, a.y);
						else
							owner_->setPosition(b.x, b.y);

						if (removeOnEnd_)
							destroyed_ = true;

						pause();
						return;
					}
				}
				
				float u = time_ / duration_;
				if (back_)
					pos = c2Lerp(b, a, u);
				else
					pos = c2Lerp(a, b, u);
				owner_->setPosition(sf::Vector2f(pos.x, pos.y));				
			}
		}

		void InterpolateToPoint::start()
		{
			running_ = true;
		}

		void InterpolateToPoint::pause()
		{
			running_ = false;
		}

		void InterpolateToPoint::reset()
		{
			time_ = 0.0f;
			back_ = false;
		}

		void InterpolateToPoint::stop()
		{
			pause();
			reset();

			owner_->setPosition(a.x, a.y);
		}

		void InterpolateToPoint::setPointStart(sf::Vector2f pt)
		{
			a = { pt.x, pt.y };
		}

		void InterpolateToPoint::setPointEnd(sf::Vector2f pt)
		{
			b = { pt.x, pt.y };
		}

		void InterpolateToPoint::setControlPoints(sf::Vector2f start, sf::Vector2f end)
		{
			a = { start.x, start.y };
			b = { end.x, end.y };
		}

		sf::Vector2f InterpolateToPoint::getPointStart()
		{
			return sf::Vector2f(a.x, a.y);
		}

		sf::Vector2f InterpolateToPoint::getPointEnd()
		{
			return sf::Vector2f(b.x, b.y);
		}

		void InterpolateToPoint::getControlPoints(sf::Vector2f & start, sf::Vector2f & end)
		{
			start = {a.x, a.y};
			end = { b.x, b.y };
		}

	}
}
