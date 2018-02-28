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
		
		// states to be drawn in the background, this has to be handled manually (remove from vector when gamestate is destroyed)
		// (todo: use shared_ptr in tgf::Game for the states to be shared here)
		std::vector<tgf::GameStateBase*> backgroundstates_;

	protected:
		void onResume();
		void onRestart();
		void onQuitToMainMenu();

		std::unique_ptr<tgf::gui::TextMenu> menu_;
		sf::Font font_;
		sf::RectangleShape pause_grey_;
	};
}