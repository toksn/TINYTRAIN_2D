#include "GameState_Pause.h"
#include "GameState_Running.h"
#include "Game.h"


namespace tinytrain
{
	GameState_Pause::GameState_Pause(tgf::Game* game)
	{
		game_ = game;

		font_.loadFromFile("data/fonts/pixantiqua.ttf");
		
		menu_ = std::make_unique<tgf::gui::TextMenu>();

		if (menu_)
		{
			menu_->appendItem(sf::Text("resume", font_), std::bind(&GameState_Pause::onResume, this));
			menu_->appendItem(sf::Text("options", font_), nullptr);
			menu_->appendItem(sf::Text("mainmenu", font_), std::bind(&GameState_Pause::onQuitToMainMenu, this));

			bindEventCallback(sf::Event::EventType::MouseMoved, menu_.get(), &tgf::gui::TextMenu::onMouseMove);
			bindEventCallback(sf::Event::EventType::MouseButtonPressed, menu_.get(), &tgf::gui::TextMenu::onMousePressed);
			bindEventCallback(sf::Event::EventType::KeyPressed, menu_.get(), &tgf::gui::TextMenu::onKeyPressed);

			if (game && game->window_)
			{
				//onWindowSizeChanged(game->window_->getSize().x, game->window_->getSize().y);
				float w = game->window_->getSize().x;
				float h = game->window_->getSize().y;
				float heightfactor = 0.7f;
				float widthfactor = 0.9f;

				sf::FloatRect size(w*(1.0f - widthfactor) / 2.0f, h* (1.0f - heightfactor) / 2.0f, w*widthfactor, h * heightfactor);
				menu_->setArea(size);
			}
		}
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
		if (game_ && game_->window_ && menu_)
		{
			target->setView(*game_->guiView_);

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
	}
	
	void GameState_Pause::onResume()
	{
		if (game_)
			game_->popState();
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