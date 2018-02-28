#pragma once
#include "GameStateBase.h"
#include "TextMenu.h"
#include <functional>
#include <memory>

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
		//virtual void handleInput(sf::Event &e) override;
		
	protected:
		void onStart();
		void onQuit();

		std::unique_ptr<tgf::gui::TextMenu> menu_;
		sf::Font font_;
	};
}