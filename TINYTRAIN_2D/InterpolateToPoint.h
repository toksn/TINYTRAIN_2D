#pragma once
#include "Entity.h"
#include "tinyc2.h"
#include <memory>

namespace tgf
{
	namespace components
	{
		enum class MovementType
		{
			OneWay,			// A>B   (A>B)  (A>B)
			TwoWay			// A>B>A (A>B>A) (A>B>A)
		};

		class InterpolateToPoint : public Component
		{
		public:
			InterpolateToPoint();
			InterpolateToPoint(sf::Vector2f start, sf::Vector2f end, float duration, MovementType type = MovementType::OneWay, bool repeat = false, bool removeOnEnd = true);
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


			MovementType type_ = MovementType::OneWay;
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