#pragma once
#include "TLineInputInterface.h"

namespace tinytrain
{
	namespace controllers
	{
		class TConeInputComponent : public TLineInputInterface
		{
		public:
			TConeInputComponent();
			~TConeInputComponent();

			// Inherited via Component
			virtual void draw(sf::RenderTarget * target) override;
			virtual void update(float deltaTime) override;
			float getCurrentAngle();
			virtual void recalcDrawRect(int width, int height) override;

		protected:
			float input_width_;
			float sensitivity_;
			float radius_;
			float max_angle_;
			float diff_;

			float center_rate_;
		};
	}
}