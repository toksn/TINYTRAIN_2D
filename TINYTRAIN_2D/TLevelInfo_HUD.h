#pragma once
#include "Entity.h"

namespace tinytrain
{
	class TLevel;
	namespace gui
	{
		class TLevelInfo_HUD : public tgf::Entity
		{
		public:
			TLevelInfo_HUD(TLevel *level, const sf::Font & font);
			~TLevelInfo_HUD();			

			void setLevel(TLevel* level);
			void recalcHUDPositions(const int & w, const int & h);

		private:
			// Inherited via Entity
			virtual void onDraw(sf::RenderTarget * target) override;
			virtual void onUpdate(float deltaTime) override;

			TLevel* level_;
			sf::Text txtTime_;
			sf::Text txtPoints_;

			char txt_buffer_[100];
		};
	}
}