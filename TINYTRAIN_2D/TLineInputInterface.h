#pragma once
#include "Component.h"

namespace tinytrain
{
	class TPlayer;
	namespace controllers
	{
		class TLineInputInterface : public tgf::Component
		{
		public:
			//friend class TPlayer;

			// this could be in the tinytrain_input_component_interface
			virtual std::vector<sf::Vector2f> getInputLine(bool reset_line = false);
			virtual void resetInputLine(int x, int y);
			virtual void setColor(sf::Color color);

			virtual void recalcDrawRect(int width, int height);

			sf::FloatRect drawingArea_;

		protected:
			sf::VertexArray inputLine_;
			sf::RectangleShape drawingAreaShape_;
			sf::Color color_;
			TPlayer* player_;
		};
	}
}