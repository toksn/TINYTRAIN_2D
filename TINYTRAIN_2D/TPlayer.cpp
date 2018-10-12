#include "TPlayer.h"
#include "TRailTrack.h"
#include "TSingleLineInputComponent.h"
#include "TPolyLineInputComponent.h"
#include "TConeInputComponent.h"

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
		
		// input options
		//input_component_ = addNewComponent<controllers::TPolyLineInputComponent>();
		//bNormalizeDrawnLineSize_ = true;
		//bNormalizeDrawnLineRotation_ = true;
		
		
		//input_component_ = addNewComponent<controllers::TSingleLineInputComponent>();
		input_component_ = addNewComponent<controllers::TConeInputComponent>();
		bNormalizeDrawnLineSize_ = true;
		bNormalizeDrawnLineRotation_ = false;

		color_ = sf::Color::Red;
		setColor(color_);

		railcast_timeout_ = 0.5f;
		railcast_timer_ = 0.25f;// railcast_timeout_ * 0.9f;
		first_rail_ = true;
	}


	TPlayer::~TPlayer()
	{
		printf("~TPlayer, possible callback crashes now!\n");
		if (gs_)
			gs_->unbindAllCallbacks(this);
	}

	void TPlayer::onDraw(sf::RenderTarget * target)
	{
		target->draw(drawingAreaShape_);
	}

	void TPlayer::onUpdate(float deltaTime)
	{
		if (inputstate_ != INPUTSTATE::IDLE && gs_ && gs_->game_)
		{
			railcast_timer_ += deltaTime;
			if (railcast_timer_ >= railcast_timeout_)
			{
				// make timeout shorter until min
				const float railcast_timeout_min = 0.25f;
				if (railcast_timeout_ > railcast_timeout_min)
				{
					railcast_timeout_ *= 0.8f;
					if (railcast_timeout_ < railcast_timeout_min)
						railcast_timeout_ = railcast_timeout_min;
				}

				// add current inputline
				if (addInputLineToRailTrack())
				{
					first_rail_ = false;
					railcast_timer_ = 0.0f;
				}
			}

			// get current mouse location
			auto curScreenPos = sf::Mouse::getPosition(*gs_->game_->window_);
			if (drawingArea_.contains(curScreenPos.x, curScreenPos.y))
			{
				// waited for drawing to begin -> new state is DRAWING
				if (inputstate_ == INPUTSTATE::DRAWING_WAIT)
					startDrawing(curScreenPos.x, curScreenPos.y);
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

		input_component_->init();
	}

	void TPlayer::startDrawing(int x, int y)
	{
		//printf("DRAWING : started at pixel %i, %i\n", x, y);

		input_component_->resetInputLine(x, y);
		
		inputstate_ = INPUTSTATE::DRAWING;
	}

	void TPlayer::stopDrawing()
	{
		if (inputstate_ == INPUTSTATE::DRAWING)
		{
			if(first_rail_)
				//printf("IDLE\t: stopped drawing, trying to add the points to the railtrack\n");
				addInputLineToRailTrack();
		}
		railcast_timer_ = 0.25f;
		railcast_timeout_ = 0.5f;
		first_rail_ = true;
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
		else if(e.mouseButton.button == sf::Mouse::Right)
		{
			if (railtrack_ && railtrack_->undo())
			{
				// apply penalty:
				// todo: speed up the train for a second
			}
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
		if(input_component_)
			input_component_->setColor(col);
	}

	bool TPlayer::addInputLineToRailTrack()
	{
		bool rc = false;
		if (railtrack_)
		{
			//std::vector<sf::Vector2f> splinePointsToAdd = input_component_->convertDrawnLineToRailTrack(railtrack_, drawingArea_);
			auto line = input_component_->getInputLine(false);
			float radius = 10.0f;
			if (line.size() == 1)
			{
				line.push_back(sf::Vector2f(line[0].x, line[0].y - radius));
			}
			else if(line.size() == 0)
			{
				line.push_back(sf::Vector2f(0.0f, 0.0f));
				line.push_back(sf::Vector2f(0.0f, -radius));
			}

			std::vector<sf::Vector2f> splinePointsToAdd = convertLineToRailTrack(line);
			//std::vector<sf::Vector2f> splinePointsToAdd = castRailTrack(input_component_->getInputLine(false));

			if (splinePointsToAdd.size())
				rc = true;

			railtrack_->addDrawnLinePoints(splinePointsToAdd);

			bool rotateCameraWithSpline = false;
			if (rotateCameraWithSpline && gs_ && splinePointsToAdd.size())
			{
				// get angle in degrees
				float angle = 0.0f;
				auto track = railtrack_->getTrackSpline();
				if (track)
					angle = track->getDirectionAngleAtTime(1.0, false) + 90.0f;

				gs_->moveCameraToPoint(splinePointsToAdd.back(), angle, 0.5f);
			}
		}

		return rc;
	}

	std::vector<sf::Vector2f> TPlayer::castRailTrack(std::vector<sf::Vector2f>& line)
	{
		const float radius = 10.0f;
		const float sensitivity = 2.5f;
		if (line.size() >= 2)
		{
			line.resize(2);
			auto pt_a = line[0];
			auto pt_b = line[1];

			float diff = pt_a.x - pt_b.x;
			diff *= 2.0f;
			diff *= sensitivity;
			diff /= drawingArea_.width;

			const float max_angle = 45.0f;
			float cur_angle = max_angle * diff;
			cur_angle *= DEG_TO_RAD;
			
			line[1].x = pt_a.x - sin(cur_angle) * radius;
			line[1].y = pt_a.y - cos(cur_angle) * radius;
		}
		else if (line.size() == 1)
		{
			line.push_back(sf::Vector2f(line[0].x, line[0].y - radius));
		}
		else
		{
			line.push_back(sf::Vector2f(0.0f, 0.0f));
			line.push_back(sf::Vector2f(0.0f, -radius));
		}

		return convertLineToRailTrack(line);
	}

	std::vector<sf::Vector2f> TPlayer::convertLineToRailTrack(std::vector<sf::Vector2f>& line)
	{
		std::vector<sf::Vector2f> splinePointsToAdd;
		if (railtrack_ && line.size())
		{
			c2v lastSplinePoint, curSquarePt;
			c2r splineDir;

			sf::Vector2f start, end;
			float angle_rad = 0.0f;
			if (railtrack_->getLastControlPointSegmentFromTrack(start, end) == false)
			{
				// at least one point is needed to continue
				if (railtrack_->getLastControlPointFromTrack(end) == false)
					return splinePointsToAdd;
			}
			else
			{
				c2v seg = c2Sub({ end.x,end.y }, { start.x,start.y });

				// correct SFML to mathematical coordinate system by rotating 90degrees
				seg = c2Skew(seg);

				angle_rad = atan2(seg.y, seg.x);
			}

			// normalize rotation: calc drawn_angle from first drawn segment
			if (bNormalizeDrawnLineRotation_ && line.size() >= 2)
			{
				c2v seg = c2Sub({ line[1].x,line[1].y }, { line[0].x,line[0].y });

				// correct SFML to mathematical coordinate system by rotating 90degrees
				seg = c2Skew(seg);

				angle_rad -= atan2(seg.y, seg.x);
			}
			splineDir = c2Rot(angle_rad);

			lastSplinePoint = { end.x, end.y };

			// *********** 1:
			// normalize on-screen square in screenspace
			// (2D bounding box -> longest side to be 1.0)
			c2v min{ drawingArea_.left, drawingArea_.top };
			c2v max{ drawingArea_.left + drawingArea_.width, drawingArea_.top + drawingArea_.height };
			c2v lineBoundingBox_Dimension, newSquare;
			float squareLengthPx = 1.0;

			if (bNormalizeDrawnLineSize_)
			{
				min = { line[0].x, line[0].y };
				max = { line[0].x, line[0].y };
				for (int i = 1; i < line.size(); i++)
				{
					c2v pt{ line[i].x, line[i].y };
					min = c2Minv(pt, min);
					max = c2Maxv(pt, max);
				}
			}
			lineBoundingBox_Dimension = c2Sub(max, min);
			squareLengthPx = c2Max(lineBoundingBox_Dimension.x, lineBoundingBox_Dimension.y);

			// normalize the screenpositions
			for (int i = 0; i < line.size(); i++)
			{
				line[i] -= sf::Vector2f(min.x, min.y);
				line[i] /= squareLengthPx;
			}


			// *********** 2:
			// repos on-screen square to have origin in first linepoint
			sf::Vector2f initPos = line[0];
			for (int i = 0; i < line.size(); i++)
				line[i] -= initPos;

			// *********** 3:
			// multiply with direction and position the points in relation to the lastSplinePoint
			for (int i = 1; i < line.size(); i++)
			{
				curSquarePt.x = line[i].x;
				curSquarePt.y = line[i].y;

				// apply segment length scale
				float splineSegmentLength = railtrack_->getSegmentLength();
				curSquarePt = c2Mulvs(curSquarePt, splineSegmentLength);

				// rotate to match spline direction
				curSquarePt = c2Mulrv(splineDir, curSquarePt);

				// position it in relation to the last square point
				curSquarePt = c2Add(curSquarePt, lastSplinePoint);

				splinePointsToAdd.push_back(sf::Vector2f(curSquarePt.x, curSquarePt.y));
			}
		}
	
		return splinePointsToAdd;
	}

}