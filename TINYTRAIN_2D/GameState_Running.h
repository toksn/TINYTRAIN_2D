#pragma once
#include <memory>
#include "GameStateBase.h"
namespace tinytrain
{
	class TLevel;

	class GameState_Running : public tgf::GameStateBase
	{
	public:
		GameState_Running(tgf::Game* game);
		~GameState_Running();

		// Inherited via GameStateBase
		virtual void update(float deltaTime) override;
		virtual void draw(sf::RenderTarget * target) override;
		virtual void handleInput(sf::Event& e) override;
		virtual void onWindowSizeChanged(int w, int h) override;

	private:
		std::unique_ptr<TLevel> m_level;
		std::unique_ptr<sf::View> m_view;
	};
}