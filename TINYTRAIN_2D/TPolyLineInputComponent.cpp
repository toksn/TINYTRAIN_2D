#include "TPolyLineInputComponent.h"
#include "TPlayer.h"
//#include "Game.h"
#include "TRailTrack.h"
#include "tinyc2.h"

namespace tinytrain
{
	namespace controllers
	{
		TPolyLineInputComponent::TPolyLineInputComponent()
		{
			minDist_ = 15.0f;
			inputLine_.setPrimitiveType(sf::PrimitiveType::LineStrip);
			player_ = nullptr;
		}

		TPolyLineInputComponent::~TPolyLineInputComponent()
		{
		}

		void TPolyLineInputComponent::draw(sf::RenderTarget * target)
		{
			target->draw(drawingAreaShape_);
			target->draw(inputLine_);
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
					if (drawingArea_.contains(curScreenPos.x, curScreenPos.y))
					{
						auto size = inputLine_.getVertexCount();
						if (size)
						{
							c2v start{ inputLine_[size - 1].position.x, inputLine_[size - 1].position.y };
							c2v end{ curScreenPos.x, curScreenPos.y };
							if (c2Len(c2Sub(end, start)) > minDist_)
								inputLine_.append(sf::Vertex(sf::Vector2f(end.x, end.y), color_));
						}
						else
						{
							inputLine_.append(sf::Vertex(sf::Vector2f(curScreenPos.x, curScreenPos.y), color_));
						}
					}
				}
			}
		}
		std::unique_ptr<tgf::Component> TPolyLineInputComponent::cloneComponent()
		{
			auto c = std::make_unique<TPolyLineInputComponent>(*this);
			c->player_ = nullptr;
			return std::move(c);
		}
	}
}