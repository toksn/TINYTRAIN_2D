#include "TLineInputInterface.h"

namespace tinytrain
{
	namespace controllers
	{
		std::vector<sf::Vector2f> TLineInputInterface::getInputLine(bool reset_line)
		{
			std::vector<sf::Vector2f> inputLine;
			if (inputLine_.getVertexCount())
			{
				sf::Vector2f last = inputLine_[0].position;
				inputLine.push_back(inputLine_[0].position);
				for (int i = 1; i < inputLine_.getVertexCount(); i++)
				{
					if(inputLine_[i].position != last)
						inputLine.push_back(inputLine_[i].position);
				}
			}
			

			if (reset_line)
				inputLine_.resize(0);

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
			drawingAreaShape_.setOutlineColor(color_);
			for (int i = 0; i < inputLine_.getVertexCount(); i++)
				inputLine_[i].color = color;
		}

		void TLineInputInterface::recalcDrawRect(int width, int height)
		{
			// factor is the part of the window height the drawing area is using (in both directions).
			//
			// example: window(800x600), factor 0.3f --> 600*0.3=180px is the size of the drawing area.
			// and it is positioned at the upper right corner at 0.1*height from both sides. 600*0.1=60px border
			float factor = 0.3f;
			sf::Vector2i pos((float)width - (float)height*(factor + 0.1f), (float)height*.1f);
			sf::Vector2i drawsize(width, height);
			drawsize.y *= factor;
			drawsize.x = drawsize.y;

			drawingArea_ = sf::FloatRect(pos.x, pos.y, drawsize.x, drawsize.y);
			drawingAreaShape_.setSize(sf::Vector2f(drawsize.x, drawsize.y));
			drawingAreaShape_.setPosition(pos.x, pos.y);
			drawingAreaShape_.setFillColor(sf::Color::Transparent);
			drawingAreaShape_.setOutlineColor(color_);
			drawingAreaShape_.setOutlineThickness(1);
		}
	}
}