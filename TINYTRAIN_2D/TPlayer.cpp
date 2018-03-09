#include "TPlayer.h"
#include "TRailTrack.h"
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
		
		// input options
		input_component_ = addNewComponent<controllers::TDirectMouseToSplineInput>();
		input_component_->bNormalizeDrawnLineSize_ = true;
		input_component_->bNormalizeDrawnLineRotation_ = true;

		color_ = sf::Color::Red;
		setColor(color_);
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
	}

	void TPlayer::startDrawing(int x, int y)
	{
		printf("DRAWING : started at pixel %i, %i\n", x, y);

		input_component_->startDrawing(x, y);
		
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
		if(input_component_)
			input_component_->setColor(col);
	}

	void TPlayer::addDrawnLineToRailTrack()
	{
		if (railtrack_)
		{
			std::vector<sf::Vector2f> splinePointsToAdd = input_component_->convertDrawnLineToRailTrack(railtrack_, drawingArea_);

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
	}



}