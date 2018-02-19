#include "GameState_Running.h"
#include <algorithm>

#include "TLevel.h"

namespace tinytrain
{

	GameState_Running::GameState_Running(tgf::Game * game)
	{
		m_game = game;
		m_level = std::make_unique<TLevel>();
		m_level->load();
	}

	GameState_Running::~GameState_Running()
	{
	}

	void GameState_Running::update(float deltaTime)
	{
		// update level
		if (m_level)
			m_level->update(deltaTime);
	}

	void GameState_Running::draw(sf::RenderTarget * target)
	{
		// gameview

		// draw level
		if (m_level)
			m_level->draw(target);

		// guiview

		// draw gui
	}

	void GameState_Running::handleInput()
	{
	}
}