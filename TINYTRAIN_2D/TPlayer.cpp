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
		gs_ = gs;
		if (gs)
		{
			gs->bindEventCallback(sf::Event::MouseButtonPressed, this, &TPlayer::onMousePressed);
			gs->bindEventCallback(sf::Event::MouseButtonReleased, this, &TPlayer::onMouseReleased);

			gs->bindEventCallback(sf::Event::KeyPressed, this, &TPlayer::onKeyPressed);
			//...
		}

		minDist_ = 15.0f;
		color_ = sf::Color::Red;
		setColor(color_);

		drawnLine_.setPrimitiveType(sf::PrimitiveType::LineStrip);
	}


	TPlayer::~TPlayer()
	{
		printf("~TPlayer, possible callback crashes now!\n");
		if (gs_)
			gs_->unbindAllCallbacks(this);
	}

	void TPlayer::draw(sf::RenderTarget * target)
	{
		Entity::draw(target);
		target->draw(drawingAreaShape_);
		target->draw(drawnLine_);
	}

	void TPlayer::update(float deltaTime)
	{
		Entity::update(deltaTime);
		if (inputstate_ != INPUTSTATE::IDLE && gs_ && gs_->game_)
		{
			// get current mouse location
			auto curScreenPos = sf::Mouse::getPosition(*gs_->game_->window_);
			if (drawingArea_.contains(curScreenPos.x, curScreenPos.y))
			{
				// waited for drawing to begin -> new state is DRAWING
				if (inputstate_ == INPUTSTATE::DRAWING_WAIT)
					startDrawing(curScreenPos.x, curScreenPos.y);
								
				auto size = drawnLine_.getVertexCount();
				if (size)
				{
					c2v start{ drawnLine_[size - 1].position.x, drawnLine_[size - 1].position.y };
					c2v end{ curScreenPos.x, curScreenPos.y };
					if (c2Len(c2Sub(end, start)) > minDist_)
						drawnLine_.append(sf::Vertex(sf::Vector2f(end.x, end.y), color_));
				}					
			}
			else if(inputstate_ == INPUTSTATE::DRAWING)
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

		drawingArea_ = sf::FloatRect(pos.x, pos.y, drawsize.x, drawsize.y);
		drawingAreaShape_.setSize(sf::Vector2f(drawsize.x, drawsize.y));
		drawingAreaShape_.setPosition(pos.x, pos.y);
		drawingAreaShape_.setFillColor(sf::Color::Transparent);
		drawingAreaShape_.setOutlineColor(color_);
		drawingAreaShape_.setOutlineThickness(1);
	}

	void TPlayer::startDrawing(int x, int y)
	{
		printf("DRAWING : started at pixel %i, %i\n", x, y);

		// store the initial point
		drawnLine_.resize(1);
		drawnLine_[0] = sf::Vertex(sf::Vector2f(x, y), color_);

		inputstate_ = INPUTSTATE::DRAWING;
	}

	void TPlayer::stopDrawing()
	{
		if (inputstate_ == INPUTSTATE::DRAWING)
		{
			printf("IDLE\t: stopped drawing, trying to add the points to the railtrack\n");
			addDrawnLineToRailTrack();
		}

		inputstate_ = INPUTSTATE::IDLE;			
	}

	void TPlayer::setTrack(TRailTrack * track)
	{
		railtrack_ = track;
		if (railtrack_ && railtrack_->getTrackSpline())
			railtrack_->getTrackSpline()->setColor(color_);
	}

	void TPlayer::onMousePressed(sf::Event& e)
	{
		if (e.mouseButton.button == sf::Mouse::Left)
		{
			if (drawingArea_.contains(e.mouseButton.x, e.mouseButton.y))
			{
				startDrawing(e.mouseButton.x, e.mouseButton.y);
			}
			else
				inputstate_ = INPUTSTATE::DRAWING_WAIT;			
		}
	}

	void TPlayer::onMouseReleased(sf::Event& e)
	{
		if (inputstate_ != INPUTSTATE::IDLE && e.mouseButton.button == sf::Mouse::Left )
		{
			stopDrawing();
		}
	}

	void TPlayer::onKeyPressed(sf::Event& e)
	{
		// PAUSE
		if (e.key.code == sf::Keyboard::Escape )
		{
			inputstate_ = INPUTSTATE::IDLE;
			gs_->pause();
		}
	}



	void TPlayer::setColor(sf::Color col)
	{
		color_ = col;
		drawingAreaShape_.setOutlineColor(color_);
		for (int i = 0; i < drawnLine_.getVertexCount(); i++)
			drawnLine_[i].color = color_;
	}

	void TPlayer::addDrawnLineToRailTrack()
	{
		if (railtrack_ && drawnLine_.getVertexCount())
		{
			std::vector<sf::Vector2f> splinePointsToAdd;
			c2v lastSplinePoint, curSquarePt;
			c2r splineDir;

			sf::Vector2f start, end;
			float angle_rad = 0.0f;
			if (railtrack_->getLastControlPointSegmentFromTrack(start, end) == false)
			{
				if (railtrack_->getLastControlPointFromTrack(end) == false)
					return;
			}
			else
			{
				c2v seg = c2Sub({ end.x,end.y }, { start.x,start.y });
				angle_rad = atan2(seg.y, seg.x);
			}			

			// calc drawn_angle from first drawn segment
			if (drawnLine_.getVertexCount() >= 2)
			{
				c2v seg = c2Sub({ drawnLine_[1].position.x,drawnLine_[1].position.y }, { drawnLine_[0].position.x,drawnLine_[0].position.y });
				angle_rad -= atan2(seg.y, seg.x);
			}
			splineDir = c2Rot(angle_rad);

			lastSplinePoint = { end.x, end.y };
			
			// *********** 1:
			// normalize on-screen square in screenspace
			// (2D bounding box -> longest side to be 1.0)
			c2v min{ drawingArea_.left, drawingArea_.top };
			c2v max{ drawingArea_.left+drawingArea_.width, drawingArea_.top+drawingArea_.height };
			c2v DrawnLine_BoundingBox_Dimension, newSquare;
			float squareLengthPx = 1.0;

			// todo: make normalizedrawnline an on/off option
			bool bNormalizeToDrawnLine = true;
			if (bNormalizeToDrawnLine)
			{
				min = { drawnLine_[0].position.x, drawnLine_[0].position.y };
				max = { drawnLine_[0].position.x, drawnLine_[0].position.y };
				for (int i = 1; i < drawnLine_.getVertexCount(); i++)
				{					
					c2v pt{ drawnLine_[i].position.x, drawnLine_[i].position.y };
					min = c2Minv(pt, min);
					max = c2Maxv(pt, max);
				}
			}			
			DrawnLine_BoundingBox_Dimension = c2Sub(max, min);
			squareLengthPx = c2Max(DrawnLine_BoundingBox_Dimension.x, DrawnLine_BoundingBox_Dimension.y);

			// normalize the screenpositions
			for (int i = 0; i < drawnLine_.getVertexCount(); i++)
			{
				drawnLine_[i].position -= sf::Vector2f(min.x, min.y);
				drawnLine_[i].position /= squareLengthPx;
			}


			// *********** 2:
			// repos on-screen square to have origin in first linepoint
			sf::Vector2f initPos = drawnLine_[0].position;//{ drawnLine_[0].position.x, drawnLine_[0].position.y };
			for (int i = 0; i < drawnLine_.getVertexCount(); i++)
				drawnLine_[i].position -= initPos;

			// *********** 3:
			// multiply with direction and position the points in relation to the lastSplinePoint
			for (int i = 1; i < drawnLine_.getVertexCount(); i++)
			{
				curSquarePt.x = drawnLine_[i].position.x;
				curSquarePt.y = drawnLine_[i].position.y;

				// apply segment length scale
				float splineSegmentLength = railtrack_->getSegmentLength();
				curSquarePt = c2Mulvs(curSquarePt, splineSegmentLength);
				
				// rotate to match spline direction
				curSquarePt = c2Mulrv(splineDir, curSquarePt);
				
				// position it in relation to the last square point
				curSquarePt = c2Add(curSquarePt, lastSplinePoint);

				splinePointsToAdd.push_back(sf::Vector2f(curSquarePt.x, curSquarePt.y));
			}

			railtrack_->addDrawnLinePoints(splinePointsToAdd);

			if (false && gs_ && splinePointsToAdd.size())
			{
				// get angle in degrees
				float angle = 0.0f;
				auto track = railtrack_->getTrackSpline();
				if (track)
					angle = track->getDirectionAngleAtTime(1.0, false) + 90.0f;

				gs_->moveCameraToPoint(splinePointsToAdd.back(), angle, 0.5f);
			}
		}
	}

}