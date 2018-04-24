#include "TSingleLineInputComponent.h"
#include "TPlayer.h"
#include "TRailTrack.h"
#include "tinyc2.h"

namespace tinytrain
{
	namespace controllers
	{
		TSingleLineInputComponent::TSingleLineInputComponent()
		{
			validDist_ = 15.0f;
			inputLine_.setPrimitiveType(sf::PrimitiveType::LineStrip);
			player_ = nullptr;
		}

		TSingleLineInputComponent::~TSingleLineInputComponent()
		{
		}

		void TSingleLineInputComponent::draw(sf::RenderTarget * target)
		{
			target->draw(inputLine_);
		}

		void TSingleLineInputComponent::update(float deltaTime)
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
						auto size = inputLine_.getVertexCount();
						if (size == 0)
							inputLine_.append(sf::Vertex(sf::Vector2f(curScreenPos.x, curScreenPos.y), color_));
						else
						{
							c2v start{ inputLine_[0].position.x, inputLine_[0].position.y };
							c2v end{ curScreenPos.x, curScreenPos.y };

							if (c2Len(c2Sub(end, start)) > validDist_)
							{
								inputLine_.resize(2);
								inputLine_[1] = sf::Vertex( sf::Vector2f{ end.x, end.y }, color_ );
							}
						}
					}
				}
			}
		}
	}
}