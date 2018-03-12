#pragma once
#include "Component.h"

namespace tinytrain
{
	class TPlayer;
	class TRailTrack;
	namespace controllers
	{
		class TPolyLineInputComponent : public tgf::Component
		{
		public:
			friend class TPlayer;

			TPolyLineInputComponent();
			~TPolyLineInputComponent();

			// Inherited via Component
			virtual void draw(sf::RenderTarget * target) override;
			virtual void update(float deltaTime) override;

			
			// this could be in the tinytrain_input_component_interface
			std::vector<sf::Vector2f> getInputLine();
			void resetInputLine(int x, int y);
			void setColor(sf::Color color);

		protected:
			TPlayer* player_;
			sf::VertexArray drawnLine_;
			
			// this could be in the tinytrain_input_component_interface
			float minDist_;
			sf::Color color_;
		};
	}
}