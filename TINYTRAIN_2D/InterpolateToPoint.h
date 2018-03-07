#pragma once
#include "Entity.h"
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
			// Inherited via Component
			virtual void draw(sf::RenderTarget * target) override;
			virtual void update(float deltaTime) override;

			virtual void start();
			virtual void pause();
			virtual void reset();
			virtual void stop();

		protected:
			bool repeat_ = false;
			
			// set to negative number to fully pause before repeat
			float delayOnRepeat = 0.0f;
			float time_ = 0.0f;
		};
	}
}