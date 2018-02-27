#include "GameState_MainMenu.h"
#include "GameState_Running.h"
#include "Game.h"


namespace tinytrain
{
	GameState_MainMenu::GameState_MainMenu(tgf::Game* game)
	{
		game_ = game;

		font_.loadFromFile("data/fonts/pixantiqua.ttf");

		auto entry = std::make_pair<std::string, tgf::GameStateBase*>("blabla1", new GameState_Running(game));
		menuentries_.push_back(entry);
	}


	GameState_MainMenu::~GameState_MainMenu()
	{
	}

	void GameState_MainMenu::update(float dt)
	{
	}

	void GameState_MainMenu::draw(sf::RenderTarget * target)
	{
		sf::Text sometext;
		sometext.setFont(font_);
		sometext.setFillColor(sf::Color::Red);

		if (game_ && game_->window_)
			target->setView(*game_->guiView_);

		for (auto e : menuentries_)
		{
			sometext.setString(e.first);
			target->draw(sometext);
		}	
	}

	void GameState_MainMenu::onWindowSizeChanged(int w, int h)
	{
	}

}