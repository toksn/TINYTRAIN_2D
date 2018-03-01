#pragma once
#include "GameStateBase.h"
#include "TextMenu.h"
#include <functional>
#include <memory>

namespace tinytrain
{
	class GameState_End : public tgf::GameStateBase
	{
	public:
		GameState_End(tgf::Game* game);
		~GameState_End();

		// Inherited via GameStateBase
		virtual void update(float dt) override;
		virtual void draw(sf::RenderTarget * target) override;
		virtual void onWindowSizeChanged(int w, int h) override;
		//virtual void handleInput(sf::Event &e) override;

		void setEndText(std::string a_, sf::Color a_color = sf::Color::White);
		
		// states to be drawn in the background, this has to be handled manually (remove from vector when gamestate is destroyed)
		// (todo: use shared_ptr in tgf::Game for the states to be shared here)
		std::vector<tgf::GameStateBase*> backgroundstates_;
		
	protected:
		void onRestart();
		void onQuitToMainMenu();

		std::unique_ptr<sf::Text> endtext_;
		std::unique_ptr<tgf::gui::TextMenu> menu_;
		sf::RectangleShape pause_grey_;
	};
}