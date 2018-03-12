#include "TPolyLineInputComponent.h"
#include "TPlayer.h"
#include "Game.h"
#include "TRailTrack.h"
#include "tinyc2.h"

namespace tinytrain
{
	namespace controllers
	{
		TPolyLineInputComponent::TPolyLineInputComponent()
		{
			minDist_ = 15.0f;
			drawnLine_.setPrimitiveType(sf::PrimitiveType::LineStrip);
			player_ = nullptr;
		}

		TPolyLineInputComponent::~TPolyLineInputComponent()
		{
		}

		void TPolyLineInputComponent::draw(sf::RenderTarget * target)
		{
			target->draw(drawnLine_);
		}

		void TPolyLineInputComponent::update(float deltaTime)
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

		std::vector<sf::Vector2f> TPolyLineInputComponent::getInputLine()
		{
			std::vector<sf::Vector2f> inputLine;
			
			for (int i = 0; i < drawnLine_.getVertexCount(); i++)
				inputLine.push_back(drawnLine_[i].position);

			return inputLine;
		}

		void TPolyLineInputComponent::resetInputLine(int x, int y)
		{
			// store the initial point
			drawnLine_.resize(1);
			drawnLine_[0] = sf::Vertex(sf::Vector2f(x, y), color_);
		}

		void TPolyLineInputComponent::setColor(sf::Color color)
		{
			color_ = color;
		}
	}
}