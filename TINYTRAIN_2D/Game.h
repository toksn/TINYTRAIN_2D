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
		GameStateBase* peekState();		

		// this can be used to manually controlling the fps instead of using the SFML framerate
		void setMaxFPS(sf::Uint16 maxFPS);

		std::unique_ptr<sf::RenderWindow> m_window;
		std::vector<std::unique_ptr<GameStateBase>> m_states;

		// gui view that can be used by every gamestate
		std::unique_ptr<sf::View> m_guiView;
	private:
		void handleGlobalInput();

		bool m_bShowFPS;
		std::unique_ptr<sf::Clock> m_frameClock;

		// this can be used to manually controlling the fps instead of using the SFML framerate
		sf::Uint16	m_maxFPS;
		sf::Time	m_renderTimer;
		sf::Time	m_desiredFrameTime;
	};

}