#include "TPlayer.h"
#include "GameState_Running.h"
#include "Game.h"

#include "tinyc2.h"

namespace tinytrain
{
	TPlayer::TPlayer(GameState_Running* gs)
	{
		m_gs = gs;
		if (gs)
		{
			gs->bindEventCallback(sf::Event::MouseButtonPressed, this, &TPlayer::onMousePressed);
			gs->bindEventCallback(sf::Event::MouseButtonReleased, this, &TPlayer::onMouseReleased);
			//...
		}

		m_minDist = 5.0f;
		m_color = sf::Color::Red;
		setColor(m_color);

		m_drawnLine.setPrimitiveType(sf::PrimitiveType::LineStrip);
	}


	TPlayer::~TPlayer()
	{
	}

	void TPlayer::draw(sf::RenderTarget * target)
	{
		target->draw(m_drawingAreaShape);
		target->draw(m_drawnLine);
	}

	void TPlayer::update(float deltaTime)
	{
		if (m_inputstate == INPUTSTATE::DRAWING && m_gs && m_gs->m_game)
		{
			// get current mouse location
			auto curScreenPos = sf::Mouse::getPosition(*m_gs->m_game->m_window);
			if (m_drawingArea.contains(curScreenPos.x, curScreenPos.y))
			{
				auto size = m_drawnLine.getVertexCount();
				if (size)
				{
					c2v start{ m_drawnLine[size - 1].position.x, m_drawnLine[size - 1].position.y };
					c2v end{ curScreenPos.x, curScreenPos.y };
					if (c2Len(c2Sub(end, start)) > m_minDist)
						m_drawnLine.append(sf::Vertex(sf::Vector2f(end.x, end.y), m_color));
				}
			}
			else
			{
				// stop drawing
				stopDrawing();
			}
		}
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

	void TPlayer::stopDrawing()
	{
		m_inputstate = INPUTSTATE::IDLE;
	}

	void TPlayer::setTrack(TRailTrack * track)
	{
		m_railtrack = track;
	}

	void TPlayer::onMousePressed(sf::Event& e)
	{
		if (e.mouseButton.button == sf::Mouse::Left && m_drawingArea.contains(e.mouseButton.x, e.mouseButton.y))
		{
			printf("DRAWING : started at pixel %i, %i\n", e.mouseButton.x, e.mouseButton.y);

			// store the initial point
			m_drawnLine.resize(1);
			m_drawnLine[0] = sf::Vertex(sf::Vector2f(e.mouseButton.x, e.mouseButton.y), m_color);

			m_inputstate = INPUTSTATE::DRAWING;
		}
	}

	void TPlayer::onMouseReleased(sf::Event& e)
	{
		if (m_inputstate != INPUTSTATE::IDLE && e.mouseButton.button == sf::Mouse::Left )
		{
			printf("IDLE\t: stopped at pixel %i, %i\n", e.mouseButton.x, e.mouseButton.y);
			stopDrawing();
		}
	}

	void TPlayer::setColor(sf::Color col)
	{
		m_color = col;
		m_drawingAreaShape.setOutlineColor(m_color);
		for (int i = 0; i < m_drawnLine.getVertexCount(); i++)
			m_drawnLine[i].color = m_color;
	}

}