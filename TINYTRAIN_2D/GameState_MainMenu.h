#pragma once
#include "GameStateBase.h"

namespace tinytrain
{
	class GameState_MainMenu : public tgf::GameStateBase
	{
	public:
		GameState_MainMenu(tgf::Game* game);
		~GameState_MainMenu();

		// Inherited via GameStateBase
		virtual void update(float dt) override;
		virtual void draw(sf::RenderTarget * target) override;
		virtual void onWindowSizeChanged(int w, int h) override;

	protected:
		std::vector<std::pair<sf::Text, tgf::GameStateBase*>> menuentries_;
		sf::Font font_;
	};
}

