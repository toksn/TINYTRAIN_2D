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
			virtual std::unique_ptr<tgf::Component> cloneComponent() override;

		protected:
			float validDist_;

			
		};
	}
}