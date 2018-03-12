#pragma once
#include "TLineInputInterface.h"

namespace tinytrain
{
	namespace controllers
	{
		class TSingleLineInputComponent : public TLineInputInterface
		{
		public:
			TSingleLineInputComponent();
			~TSingleLineInputComponent();

			// Inherited via Component
			virtual void draw(sf::RenderTarget * target) override;
			virtual void update(float deltaTime) override;

		protected:
			float validDist_;
		};
	}
}