#include "GameState_Running.h"
#include <algorithm>

#include "TLevel.h"
#include "TPlayer.h"
#include "TTrain.h"
#include "Game.h"

namespace tinytrain
{
	GameState_Running::GameState_Running(tgf::Game * game)
	{
		game_ = game;
		level_ = std::make_unique<TLevel>();
		level_->load();

		player_ = std::make_unique<TPlayer>(this);

		camera_ = std::make_unique<sf::View>();
		if (game && camera_ && game->window_)
		{
			sf::Vector2f size = sf::Vector2f(game->window_->getSize());
			camera_->setSize(size);
			camera_->setCenter(size*0.5f);

			player_->recalcDrawRect(size.x, size.y);
			if (level_)
				player_->setTrack(level_->railtrack_.get());
		}
	}

	GameState_Running::~GameState_Running()
	{
	}

	void GameState_Running::update(float deltaTime)
	{
		if (level_)
		{
			// update level
			level_->update(deltaTime);

			// update view 
			if (level_->train_ && camera_)
				camera_->setCenter(level_->train_->getPosition());

			// update player (mouse input to spline)
			if (player_)
				player_->update(deltaTime);
		}
			
	}

	void GameState_Running::draw(sf::RenderTarget * target)
	{
		if (target == nullptr)
			return;

		// gameview
		target->setView(*camera_);

		// draw level
		if (level_)
			level_->draw(target);
		
		// guiview
		if (game_ && game_->window_)
			target->setView(*game_->guiView_);

		// draw gui
		
		// draw player (drawing rect)
		if(player_)
			player_->draw(target);

		// destroying the player on purpose (for testing purposes)
		//player_.reset(nullptr);
	}

	/*
	void GameState_Running::handleInput(sf::Event& e)
	{
		for (auto f : eventCallbacks_[e.type])
		{
			f(e);
		}
	}*/

	void GameState_Running::onWindowSizeChanged(int w, int h)
	{
		if (camera_)
			camera_->setSize(w, h);

		player_->recalcDrawRect(w, h);
	}
}