#include "TConeInputComponent.h"
#include "TPlayer.h"
#include "TRailTrack.h"
#include "tinyc2.h"

namespace tinytrain
{
	namespace controllers
	{
		TConeInputComponent::TConeInputComponent()
		{
			input_width_ = 200.0f;
			sensitivity_ = 1.0f;//2.5f;
			radius_ = 10.0f;
			max_angle_ = 45.0f;
			diff_ = 0.0f;
			inputLine_.setPrimitiveType(sf::PrimitiveType::LineStrip);
			player_ = nullptr;
		}

		TConeInputComponent::~TConeInputComponent()
		{
		}

		void TConeInputComponent::draw(sf::RenderTarget * target)
		{
			

			target->draw(inputLine_);
		}

		void TConeInputComponent::update(float deltaTime)
		{
			//if(player_ == nullptr && owner_ != nullptr)
			if (player_ != owner_)
			{
				player_ = dynamic_cast<TPlayer*>(owner_);
				init();					
			}

			if (player_ && player_->inputstate_ == INPUTSTATE::DRAWING)
			{
				if (player_->gs_ && player_->gs_->game_)
				{
					// get current mouse location
					auto curScreenPos = sf::Mouse::getPosition(*player_->gs_->game_->window_);
					auto size = inputLine_.getVertexCount();
					if (size < 2)
					{
						inputLine_.append(sf::Vertex(sf::Vector2f(curScreenPos.x, curScreenPos.y), color_));
						diff_ = 0.0f;
					}
					else
					{
						inputLine_.resize(2);
						auto pt_a = inputLine_[0].position;
						//sf::Vector2f pt_b{ (float)curScreenPos.x, (float)curScreenPos.y };

						float cur_diff = pt_a.x - curScreenPos.x;
						cur_diff *= 2.0f;
						cur_diff *= sensitivity_;
						cur_diff /= input_width_;

						if (cur_diff == 0.0f)
						{
							// turn back slower the less it was moved
							//diff_ *= 1.0f - 1.0f * deltaTime;

							// turn back linear 0.7 of full diff per sec
							float turn_back = 0.7f * deltaTime;
							float new_diff = diff_ < 0.0f ? -diff_ : diff_;
							
							new_diff -= turn_back;
							new_diff = new_diff < 0.0f ? 0.0f : new_diff;

							diff_ = diff_ < 0.0f ? -new_diff : new_diff;
						}
						else
							diff_ += cur_diff;

						// keep in -1 to +1 range
						diff_ = diff_ > 1.0f ? 1.0f : diff_;
						diff_ = diff_ < -1.0f ? -1.0f : diff_;
						
						float cur_angle = max_angle_ * diff_;
						cur_angle *= DEG_TO_RAD;

						inputLine_[1].position.x = pt_a.x - sin(cur_angle) * radius_;
						inputLine_[1].position.y = pt_a.y - cos(cur_angle) * radius_;

						// set mouse back
						sf::Mouse::setPosition(sf::Vector2i(pt_a.x, pt_a.y), *player_->gs_->game_->window_);
					}
				}
			}
		}

		void TConeInputComponent::init()
		{
			if (player_)
			{
				input_width_ = player_->drawingArea_.width;

				// trigonometry
				float a = input_width_ * 0.5f;
				float alpha = max_angle_ * DEG_TO_RAD;
				radius_ = a / sin(alpha);
			}
		}
	}
}