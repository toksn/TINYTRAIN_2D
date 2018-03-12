#include "TLineInputInterface.h"

namespace tinytrain
{
	namespace controllers
	{
		std::vector<sf::Vector2f> TLineInputInterface::getInputLine()
		{
			std::vector<sf::Vector2f> inputLine;

			for (int i = 0; i < inputLine_.getVertexCount(); i++)
				inputLine.push_back(inputLine_[i].position);

			return inputLine;
		}

		void TLineInputInterface::resetInputLine(int x, int y)
		{
			// store the initial point
			inputLine_.resize(1);
			inputLine_[0] = sf::Vertex(sf::Vector2f(x, y), color_);
		}

		void TLineInputInterface::setColor(sf::Color color)
		{
			color_ = color;
			for (int i = 0; i < inputLine_.getVertexCount(); i++)
				inputLine_[i].color = color;
		}
	}
}