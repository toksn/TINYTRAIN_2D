#include "GameState_MainMenu.h"
#include "GameState_Running.h"
#include "Game.h"


namespace tinytrain
{
	GameState_MainMenu::GameState_MainMenu(tgf::Game* game)
	{
		game_ = game;

		font_.loadFromFile("data/fonts/pixantiqua.ttf");
		
		menu_ = std::make_unique<tgf::gui::TextMenu>();

		if (menu_)
		{
			menu_->appendItem(sf::Text("start", font_), std::bind(&GameState_MainMenu::onStart, this));
			menu_->appendItem(sf::Text("options", font_), std::bind(&GameState_MainMenu::onStart, this));
			menu_->appendItem(sf::Text("blabla12354789+$%\"(§", font_), std::bind(&GameState_MainMenu::onStart, this));
			menu_->appendItem(sf::Text("thelazybrownfox", font_), std::bind(&GameState_MainMenu::onStart, this));
			menu_->appendItem(sf::Text("jumpsovertheidontknowwhat", font_), std::bind(&GameState_MainMenu::onStart, this));
			menu_->appendItem(sf::Text("quit", font_), std::bind(&GameState_MainMenu::onQuit, this));

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


	GameState_MainMenu::~GameState_MainMenu()
	{
	}

	void GameState_MainMenu::update(float dt)
	{
		if(menu_)
			menu_->update(dt);
	}

	void GameState_MainMenu::draw(sf::RenderTarget * target)
	{
		if (game_ && game_->window_ && menu_)
		{
			target->setView(*game_->guiView_);

			menu_->draw(target);
		}
	}

	void GameState_MainMenu::onWindowSizeChanged(int w, int h)
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
	
	void GameState_MainMenu::onStart()
	{
		if (game_)
		{
			// BUG:
			// game->changestate can lead to a problem because it is essentially suicide for this gamestate.
			// Combined with the fact that onStart is called in reaction to the eventcallbacks loop (GameStateBase handleInput) 
			// the loop gets invalid and crashes after the suicide.
			//
			// game_->changeState(std::make_unique<tinytrain::GameState_Running>(game_));

			game_->states_.push_back(std::make_unique<tinytrain::GameState_Running>(game_));
		}
	}

	void GameState_MainMenu::onQuit()
	{
		if (game_ && game_->window_)
			game_->window_->close();
	}

}