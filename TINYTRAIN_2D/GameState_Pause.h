#pragma once
#include "GameStateBase.h"
#include "TextMenu.h"
#include <functional>
#include <memory>

namespace tinytrain
{
	class GameState_Pause : public tgf::GameStateBase
	{
	public:
		GameState_Pause(tgf::Game* game);
		~GameState_Pause();

		// Inherited via GameStateBase
		virtual void update(float dt) override;
		virtual void draw(sf::RenderTarget * target) override;
		virtual void onWindowSizeChanged(int w, int h) override;
		//virtual void handleInput(sf::Event &e) override;
		
	protected:
		void onResume();
		void onQuitToMainMenu();

		std::unique_ptr<tgf::gui::TextMenu> menu_;
		sf::Font font_;
	};
}