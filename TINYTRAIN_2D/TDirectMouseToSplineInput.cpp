#include "TDirectMouseToSplineInput.h"
#include "TPlayer.h"
#include "Game.h"
#include "TRailTrack.h"
#include "tinyc2.h"

namespace tinytrain
{
	namespace controllers
	{
		TDirectMouseToSplineInput::TDirectMouseToSplineInput()
		{
			minDist_ = 15.0f;
			drawnLine_.setPrimitiveType(sf::PrimitiveType::LineStrip);
			player_ = nullptr;
		}

		TDirectMouseToSplineInput::~TDirectMouseToSplineInput()
		{
		}

		void TDirectMouseToSplineInput::draw(sf::RenderTarget * target)
		{
			target->draw(drawnLine_);
		}

		void TDirectMouseToSplineInput::update(float deltaTime)
		{
			//if(player_ == nullptr && owner_ != nullptr)
			if (player_ != owner_)
				player_ = dynamic_cast<TPlayer*>(owner_);

			if (player_ && player_->inputstate_ == INPUTSTATE::DRAWING)
			{
				if (player_->gs_ && player_->gs_->game_)
				{
					// get current mouse location
					auto curScreenPos = sf::Mouse::getPosition(*player_->gs_->game_->window_);
					if (player_->drawingArea_.contains(curScreenPos.x, curScreenPos.y))
					{
						auto size = drawnLine_.getVertexCount();
						if (size)
						{
							c2v start{ drawnLine_[size - 1].position.x, drawnLine_[size - 1].position.y };
							c2v end{ curScreenPos.x, curScreenPos.y };
							if (c2Len(c2Sub(end, start)) > minDist_)
								drawnLine_.append(sf::Vertex(sf::Vector2f(end.x, end.y), color_));
						}
						else
						{
							drawnLine_.append(sf::Vertex(sf::Vector2f(curScreenPos.x, curScreenPos.y), color_));
						}
					}
				}
			}
		}

		std::vector<sf::Vector2f> TDirectMouseToSplineInput::convertDrawnLineToRailTrack(TRailTrack* railtrack, sf::FloatRect drawingArea)
		{
			std::vector<sf::Vector2f> splinePointsToAdd;
			if (railtrack && drawnLine_.getVertexCount())
			{
				c2v lastSplinePoint, curSquarePt;
				c2r splineDir;

				sf::Vector2f start, end;
				float angle_rad = 0.0f;
				if (railtrack->getLastControlPointSegmentFromTrack(start, end) == false)
				{
					// at least one point is needed to continue
					if (railtrack->getLastControlPointFromTrack(end) == false)
						return splinePointsToAdd;
				}
				else
				{
					c2v seg = c2Sub({ end.x,end.y }, { start.x,start.y });
					angle_rad = atan2(seg.y, seg.x);
				}

				// normalize rotation: calc drawn_angle from first drawn segment
				if (bNormalizeDrawnLineRotation_ && drawnLine_.getVertexCount() >= 2)
				{
					c2v seg = c2Sub({ drawnLine_[1].position.x,drawnLine_[1].position.y }, { drawnLine_[0].position.x,drawnLine_[0].position.y });
					angle_rad -= atan2(seg.y, seg.x);
				}
				splineDir = c2Rot(angle_rad);

				lastSplinePoint = { end.x, end.y };

				// *********** 1:
				// normalize on-screen square in screenspace
				// (2D bounding box -> longest side to be 1.0)
				c2v min{ drawingArea.left, drawingArea.top };
				c2v max{ drawingArea.left + drawingArea.width, drawingArea.top + drawingArea.height };
				c2v drawnLine_BoundingBox_Dimension, newSquare;
				float squareLengthPx = 1.0;

				if (bNormalizeDrawnLineSize_)
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
				drawnLine_BoundingBox_Dimension = c2Sub(max, min);
				squareLengthPx = c2Max(drawnLine_BoundingBox_Dimension.x, drawnLine_BoundingBox_Dimension.y);

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
					float splineSegmentLength = railtrack->getSegmentLength();
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

		void TDirectMouseToSplineInput::startDrawing(int x, int y)
		{
			// store the initial point
			drawnLine_.resize(1);
			drawnLine_[0] = sf::Vertex(sf::Vector2f(x, y), color_);
		}

		void TDirectMouseToSplineInput::setColor(sf::Color color)
		{
			color_ = color;
		}
	}
}