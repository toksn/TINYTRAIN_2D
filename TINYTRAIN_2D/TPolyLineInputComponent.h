#pragma once
#include "TLineInputInterface.h"

namespace tinytrain
{
	namespace controllers
	{
		class TPolyLineInputComponent : public TLineInputInterface
		{
		public:
			TPolyLineInputComponent();
			~TPolyLineInputComponent();

			// Inherited via Component
			virtual void draw(sf::RenderTarget * target) override;
			virtual void update(float deltaTime) override;
		protected:
			float minDist_;
		};
	}
}