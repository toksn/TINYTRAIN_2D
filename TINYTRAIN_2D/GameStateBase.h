#pragma once
#include <SFML\Graphics.hpp>
class Game;

namespace tgf
{
	class GameStateBase
	{
	public:
		virtual void update(float dt) = 0;
		virtual void draw(sf::RenderTarget* target) = 0;
		virtual void handleInput() = 0;

		Game* m_game = 0;
	};
}