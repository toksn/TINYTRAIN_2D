#pragma once
#include <SFML\Graphics.hpp>

namespace tgf
{
	class Game;

	class GameStateBase
	{
	public:
		virtual void update(float dt) = 0;
		virtual void draw(sf::RenderTarget * target) = 0;
		virtual void handleInput(sf::Event& e) = 0;

		virtual void onWindowSizeChanged(int w, int h) = 0;

		Game* m_game = 0;
	};
}