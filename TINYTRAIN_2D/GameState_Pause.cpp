#include "GameState_Pause.h"
#include "GameState_Running.h"
#include "Game.h"


namespace tinytrain
{
	GameState_Pause::GameState_Pause(tgf::Game* game)
	{
		game_ = game;
		
		menu_ = std::make_unique<tgf::gui::TextMenu>();

		if (menu_ && game_)
		{
			menu_->maxEntryHeight_ = 100;

			menu_->appendItem(sf::Text("resume", *game_->font_), std::bind(&GameState_Pause::onResume, this));
			menu_->appendItem(sf::Text("restart", *game_->font_), std::bind(&GameState_Pause::onRestart, this));
			menu_->appendItem(sf::Text("options", *game_->font_), nullptr);
			menu_->appendItem(sf::Text("mainmenu", *game_->font_), std::bind(&GameState_Pause::onQuitToMainMenu, this));

			bindEventCallback(sf::Event::EventType::MouseMoved, menu_.get(), &tgf::gui::TextMenu::onMouseMove);
			bindEventCallback(sf::Event::EventType::MouseButtonPressed, menu_.get(), &tgf::gui::TextMenu::onMousePressed);
			bindEventCallback(sf::Event::EventType::KeyPressed, menu_.get(), &tgf::gui::TextMenu::onKeyPressed);

			if (game->window_)
			{
				onWindowSizeChanged(game->window_->getSize().x, game->window_->getSize().y);
			}
		}

		pause_grey_.setFillColor(sf::Color(0, 0, 0, 200));
	}

	GameState_Pause::~GameState_Pause()
	{
	}

	void GameState_Pause::update(float dt)
	{
		if(menu_)
			menu_->update(dt);
	}

	void GameState_Pause::draw(sf::RenderTarget * target)
	{
		// draw background gamestates
		for (auto state : backgroundstates_)
			state->draw(target);

		if (game_ && game_->window_ && menu_)
		{
			target->setView(*game_->guiView_);

			if (backgroundstates_.size())
				target->draw(pause_grey_);

			menu_->draw(target);
		}
	}

	void GameState_Pause::onWindowSizeChanged(int w, int h)
	{
		if (menu_)
		{
			// factor of the full height and width used for the menu
			float heightfactor = 0.7f;
			float widthfactor = 0.9f;

			sf::FloatRect size(w*(1 - widthfactor) / 2, h* (1 - heightfactor) / 2, w*widthfactor, h * heightfactor);
			menu_->setArea(size);
		}

		pause_grey_.setSize(sf::Vector2f(w, h));
	}
		
	void GameState_Pause::onResume()
	{
		if (game_)
			game_->popState();
	}

	void GameState_Pause::onRestart()
	{
		for (auto& state : backgroundstates_)
		{
			auto gs = dynamic_cast<GameState_Running*>(state);
			if (gs != nullptr)
			{
				gs->restart();
			}
		}

		// resume the game
		onResume();
	}

	void GameState_Pause::onQuitToMainMenu()
	{
		if (game_)
		{
			// remove pause and running state
			// todo: remove states until mainmenu
			game_->popState();
			game_->popState();
		}
	}

}