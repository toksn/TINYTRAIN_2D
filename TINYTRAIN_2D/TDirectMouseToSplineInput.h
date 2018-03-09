#pragma once
#include "Component.h"
#include "TPlayer.h"

namespace tinytrain
{
	namespace controllers
	{
		class TDirectMouseToSplineInput : public tgf::Component
		{
		public:
			friend class TPlayer;

			TDirectMouseToSplineInput();
			~TDirectMouseToSplineInput();

			// Inherited via Component
			virtual void draw(sf::RenderTarget * target) override;
			virtual void update(float deltaTime) override;

		protected:
			TPlayer* player_;
			float minDist_;
			sf::Color color_;
		};
	}
}