#include "TPlayer.h"
#include "TRailTrack.h"
#include "GameState_Running.h"
#include "Game.h"

#include "tinyc2.h"

namespace tinytrain
{
#pragma warning(disable:4244)
#pragma warning(disable:4838)

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
		printf("~TPlayer, possible callback crashes now!\n");
		if (m_gs)
			m_gs->unbindAllCallbacks(this);
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
		printf("IDLE\t: stopped drawing, trying to add the points to the railtrack\n");
		m_inputstate = INPUTSTATE::IDLE;
		addDrawnLineToRailTrack();
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

	void TPlayer::addDrawnLineToRailTrack()
	{
		if (m_railtrack && m_drawnLine.getVertexCount())
		{
			std::vector<sf::Vector2f> splinePointsToAdd;
			c2v lastSplinePoint, curSquarePt;
			c2r splineDir;

			auto pt = m_railtrack->getLocationAtTime(1.0);
			lastSplinePoint.x = pt.x;
			lastSplinePoint.y = pt.y;

			float angle_rad = m_railtrack->getDirectionAngleAtTime(1.0);
			splineDir = c2Rot(angle_rad);
			
			// *********** 1:
			// normalize on-screen square in screenspace
			// (2D bounding box -> longest side to be 1.0)
			c2v min{ m_drawingArea.left, m_drawingArea.top };
			c2v max{ m_drawingArea.left+m_drawingArea.width, m_drawingArea.top+m_drawingArea.height };
			c2v DrawnLine_BoundingBox_Dimension, newSquare;
			float squareLengthPx = 1.0;

			// todo: make normalizedrawnline = false an option
			bool bNormalizeToDrawnLine = true;
			if (bNormalizeToDrawnLine)
			{
				min = { m_drawnLine[0].position.x, m_drawnLine[0].position.y };
				max = { m_drawnLine[0].position.x, m_drawnLine[0].position.y };
				for (int i = 1; i < m_drawnLine.getVertexCount(); i++)
				{					
					c2v pt{ m_drawnLine[i].position.x, m_drawnLine[i].position.y };
					min = c2Minv(pt, min);
					max = c2Maxv(pt, max);
				}
			}			
			DrawnLine_BoundingBox_Dimension = c2Sub(max, min);
			squareLengthPx = c2Max(DrawnLine_BoundingBox_Dimension.x, DrawnLine_BoundingBox_Dimension.y);

			// normalize the screenpositions
			for (int i = 0; i < m_drawnLine.getVertexCount(); i++)
			{
				m_drawnLine[i].position -= sf::Vector2f(min.x, min.y);
				m_drawnLine[i].position /= squareLengthPx;
			}


			// *********** 2:
			// repos on-screen square to have origin in first linepoint
			sf::Vector2f initPos = m_drawnLine[0].position;//{ m_drawnLine[0].position.x, m_drawnLine[0].position.y };
			for (int i = 0; i < m_drawnLine.getVertexCount(); i++)
			{
				m_drawnLine[i].position -= initPos;

				// rotate by 90 degree to fit SFML orientation
				float temp = m_drawnLine[i].position.x;
				m_drawnLine[i].position.x = -m_drawnLine[i].position.y;
				m_drawnLine[i].position.y = temp;
			}

			// *********** 3:
			// multiply with direction and position the points in relation to the lastSplinePoint
			for (int i = 1; i < m_drawnLine.getVertexCount(); i++)
			{
				curSquarePt.x = m_drawnLine[i].position.x;
				curSquarePt.y = m_drawnLine[i].position.y;

				// apply segment length scale
				float splineSegmentLength = m_railtrack->getSegmentLength();
				curSquarePt = c2Mulvs(curSquarePt, splineSegmentLength);
				
				// rotate to match spline direction
				curSquarePt = c2Mulrv(splineDir, curSquarePt);
				
				// position it in relation to the last square point
				curSquarePt = c2Add(curSquarePt, lastSplinePoint);

				splinePointsToAdd.push_back(sf::Vector2f(curSquarePt.x, curSquarePt.y));
			}

			m_railtrack->addDrawnLinePoints(splinePointsToAdd, m_color);
		}
	}

}