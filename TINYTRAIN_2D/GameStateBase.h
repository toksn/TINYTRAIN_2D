#pragma once
#include <SFML\Graphics.hpp>
#include <functional>

namespace tgf
{
	class Game;

	class GameStateBase
	{
	public:
		virtual void update(float dt) = 0;
		virtual void draw(sf::RenderTarget * target) = 0;
		virtual void handleInput(sf::Event& e);

		virtual void onWindowSizeChanged(int w, int h) = 0;


		template<class T> void bindEventCallback(sf::Event::EventType et, T* const object, void(T::* const mf)(sf::Event))
		{
			m_eventCallbacks[et].push_back(std::bind(mf, object, std::placeholders::_1));
		}

		Game* m_game = 0;

	private:
		// store function pointers to call, mapped to specific events
		std::map<sf::Event::EventType, std::vector<std::function<void(sf::Event)>>> m_eventCallbacks;
	};
}