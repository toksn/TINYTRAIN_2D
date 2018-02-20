#include "TPlayer.h"
#include "GameState_Running.h"

namespace tinytrain
{
	TPlayer::TPlayer(GameState_Running* gs)
	{
		if (gs)
		{
			gs->bindEventCallback(sf::Event::MouseButtonPressed, this, &TPlayer::onMousePressed);
			//...
		}
	}


	TPlayer::~TPlayer()
	{
	}

	void TPlayer::draw(sf::RenderTarget * target)
	{
		target->draw(m_drawingAreaShape);
	}

	void TPlayer::update(float deltaTime)
	{
	}

	void TPlayer::recalcDrawRect(int width, int height)
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

		m_drawingArea = sf::FloatRect(pos.x, pos.y, drawsize.x, drawsize.y);
		m_drawingAreaShape.setSize(sf::Vector2f(drawsize.x, drawsize.y));
		m_drawingAreaShape.setPosition(pos.x, pos.y);
		m_drawingAreaShape.setFillColor(sf::Color::Transparent);
		m_drawingAreaShape.setOutlineColor(sf::Color::Red);
		m_drawingAreaShape.setOutlineThickness(1);
	}

	void TPlayer::setTrack(TRailTrack * track)
	{
		m_railtrack = track;
	}

	void TPlayer::onMousePressed(sf::Event e)
	{
		printf("mousePressed, pixel %i, %i\n", e.mouseButton.x, e.mouseButton.y);
	}

}