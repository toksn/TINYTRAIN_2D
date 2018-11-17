#pragma once
#include "Entity.h"
#include "tinyc2.h"
#include <memory>

namespace tgf
{
	namespace components
	{
		class InterpolateToPoint : public Component
		{
		public:
			InterpolateToPoint();
			InterpolateToPoint(sf::Vector2f start, sf::Vector2f end, float duration, directionMode type = directionMode::OneWay, bool repeat = false, bool removeOnEnd = true);
			~InterpolateToPoint();

			// Inherited via Component
			virtual void draw(sf::RenderTarget * target) override;
			virtual void update(float deltaTime) override;

			virtual void start();
			virtual void pause();
			virtual void reset();
			virtual void stop();

			void setPointStart(sf::Vector2f pt);
			void setPointEnd(sf::Vector2f pt);
			void setControlPoints(sf::Vector2f start, sf::Vector2f end);
			sf::Vector2f getPointStart();
			sf::Vector2f getPointEnd();
			void getControlPoints(sf::Vector2f& start, sf::Vector2f& end);


			directionMode type_ = directionMode::OneWay;
			bool repeat_ = false;
			// set to negative number to fully pause before repeat
			float delayOnRepeat_ = 0.0f;
			float duration_ = 1.0f;

			// use this to make it a "fire and forget" moving component
			bool removeOnEnd_ = true;

		protected:
			c2v a, b;
			float time_ = 0.0f;
			bool running_ = false;
			bool back_ = false;
		};
	}
}