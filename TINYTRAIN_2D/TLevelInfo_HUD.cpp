#include "TLevelInfo_HUD.h"
#include "TLevel.h"

namespace tinytrain
{
	namespace gui
	{
		TLevelInfo_HUD::TLevelInfo_HUD(TLevel* level, const sf::Font & font)
		{
			level_ = level;
			

			txtTime_.setString("");
			txtTime_.setFont(font);
			
			txtPoints_.setString("");
			txtPoints_.setFont(font);
		}
		TLevelInfo_HUD::~TLevelInfo_HUD()
		{
		}
		void TLevelInfo_HUD::setLevel(TLevel * level)
		{
			level_ = level;
		}

		void TLevelInfo_HUD::recalcHUDPositions(const int & w, const int & h)
		{
			txtTime_.setPosition(w / 2, 0);
		}

		void TLevelInfo_HUD::onDraw(sf::RenderTarget * target)
		{
			if (level_)
			{
				if (level_->info_.timelimit > 0)
				{
					// draw timelimit text
					target->draw(txtTime_);
				}

				if (level_->info_.points_to_reach > 0)
				{
					// draw x / y text
					target->draw(txtPoints_);
				}

				//target->draw(arrow_);

				if (level_->points_ >= level_->info_.points_to_reach)
				{
					// draw arrow to next destination
				}
				else 
				{
					// draw arrow to next point source
				}
			}
		}

		void TLevelInfo_HUD::onUpdate(float deltaTime)
		{
			if (level_)
			{
				if (level_->info_.timelimit > 0)
				{
					// time text
					snprintf(txt_buffer_, 100, "%3d sec", (int)(level_->info_.timelimit - level_->elapsed_time_));
					txtTime_.setString(txt_buffer_);
				}

				if (level_->info_.points_to_reach > 0)
				{
					// points text
					snprintf(txt_buffer_, 100, "%3d / %3d pts", level_->points_, level_->info_.points_to_reach);
					txtPoints_.setString(txt_buffer_);
				}

				if (level_->points_ >= level_->info_.points_to_reach)
				{
					// calc arrow to next destination
				}
				else
				{
					// calc arrow to next point source
				}
			}
		}
	}
}