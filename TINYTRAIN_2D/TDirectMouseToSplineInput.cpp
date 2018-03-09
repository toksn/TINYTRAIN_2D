#include "TDirectMouseToSplineInput.h"
#include "Game.h"
#include "tinyc2.h"

namespace tinytrain
{
	namespace controllers
	{
		TDirectMouseToSplineInput::TDirectMouseToSplineInput()
		{
			minDist_ = 15.0f;
			player_ = nullptr;
		}

		TDirectMouseToSplineInput::~TDirectMouseToSplineInput()
		{
		}

		void TDirectMouseToSplineInput::draw(sf::RenderTarget * target)
		{
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
						auto size = player_->drawnLine_.getVertexCount();
						if (size)
						{
							c2v start{ player_->drawnLine_[size - 1].position.x, player_->drawnLine_[size - 1].position.y };
							c2v end{ curScreenPos.x, curScreenPos.y };
							if (c2Len(c2Sub(end, start)) > minDist_)
								player_->appendDrawnLinePoint(sf::Vector2f(end.x, end.y)); 
						}
						else
						{
							player_->appendDrawnLinePoint(sf::Vector2f(curScreenPos.x, curScreenPos.y));
						}
					}
				}
			}
		}
	}
}