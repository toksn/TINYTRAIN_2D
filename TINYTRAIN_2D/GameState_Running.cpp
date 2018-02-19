#include "GameState_Running.h"
#include <algorithm>

#include "TLevel.h"
#include "TTrain.h"
#include "Game.h"

namespace tinytrain
{

	GameState_Running::GameState_Running(tgf::Game * game)
	{
		m_game = game;
		m_level = std::make_unique<TLevel>();
		m_level->load();

		m_view = std::make_unique<sf::View>();
		if (game && m_view && game->m_window)
		{
			sf::Vector2f size = sf::Vector2f(game->m_window->getSize());
			m_view->setSize(size);
			m_view->setCenter(size*0.5f);
		}
			
	}

	GameState_Running::~GameState_Running()
	{
	}

	void GameState_Running::update(float deltaTime)
	{
		if (m_level)
		{
			// update level
			m_level->update(deltaTime);

			// update view 
			if (m_level->m_train && m_view)
				m_view->setCenter(m_level->m_train->getPosition());
		}
			
	}

	void GameState_Running::draw(sf::RenderTarget * target)
	{
		if (target == nullptr)
			return;

		// gameview
		target->setView(*m_view);

		// draw level
		if (m_level)
			m_level->draw(target);
		/*
		// todo: guiview
		if (m_game && m_game->m_window)
			target->setView(*m_game->m_guiView);

		// draw gui
		// text something*/
	}

	void GameState_Running::handleInput(sf::Event& e)
	{
		// todo: inputs for new parts of the track
		switch (e.type)
		{
		case sf::Event::MouseMoved:
			//if(m_drawingState == DRAWINGSTATE::IDLE)
			break;
		default:
			break;
		}

		// ...
	}
	void GameState_Running::onWindowSizeChanged(int w, int h)
	{
		if (m_view)
			m_view->setSize(w, h);
	}
}