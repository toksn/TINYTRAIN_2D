#pragma once
#include <memory>
#include <stack>

#include <SFML/Graphics.hpp>

#include "GameStateBase.h"

namespace tgf
{
	class Game
	{
	public:
		Game(std::string game_name, int window_width, int window_height, int framerate_limit = 100);
		~Game();

		// actual game functions
		void run();
		void update();

		// managing states
		void changeState(std::unique_ptr<GameStateBase> state);
		void pushState(std::unique_ptr<GameStateBase> state);
		void popState();
		GameStateBase* peekState();

		// this can be used to manually controlling the fps instead of using the SFML framerate
		void setMaxFPS(sf::Uint16 maxFPS);

		std::unique_ptr<sf::RenderWindow> window_;
		

		// gui view and font that can be used by every gamestate
		std::unique_ptr<sf::View> guiView_;
		std::unique_ptr<sf::Font> font_;
	private:
		void handleGlobalInput();

		bool bShowFPS_;
		std::unique_ptr<sf::Clock> frameClock_;
		// gamestates to handle
		std::vector<std::unique_ptr<GameStateBase>> states_;		
		// this is necessary because the states can potentially delete themselves from the states_ vector and thus get deleted while in the middle of execution (eventloop)
		std::vector<std::unique_ptr<GameStateBase>> removedStates_;

		// this can be used to manually controlling the fps instead of using the SFML framerate
		sf::Uint16	maxFPS_;
		sf::Time	renderTimer_;
		sf::Time	desiredFrameTime_;
	};

}