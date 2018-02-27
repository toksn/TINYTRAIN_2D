#include "GameState_MainMenu.h"
#include "GameState_Running.h"
#include "Game.h"


namespace tinytrain
{
	GameState_MainMenu::GameState_MainMenu(tgf::Game* game)
	{
		game_ = game;

		font_.loadFromFile("data/fonts/pixantiqua.ttf");

		menuentries_.push_back(std::make_pair<sf::Text, tgf::GameStateBase*>(sf::Text("start", font_),								new GameState_Running(game)));
		menuentries_.push_back(std::make_pair<sf::Text, tgf::GameStateBase*>(sf::Text("options", font_),							new GameState_Running(game)));
		menuentries_.push_back(std::make_pair<sf::Text, tgf::GameStateBase*>(sf::Text("blabla12354789+$%\"(§", font_),				new GameState_Running(game)));
		menuentries_.push_back(std::make_pair<sf::Text, tgf::GameStateBase*>(sf::Text("thelazybrownfox", font_),					new GameState_Running(game)));
		menuentries_.push_back(std::make_pair<sf::Text, tgf::GameStateBase*>(sf::Text("jumpsovertheidontknowwhatsmissing", font_),	new GameState_Running(game)));
		menuentries_.push_back(std::make_pair<sf::Text, tgf::GameStateBase*>(sf::Text("quit", font_),								NULL));

		if (game && game->window_)
		{
			onWindowSizeChanged(game->window_->getSize().x, game->window_->getSize().y);
		}
	}


	GameState_MainMenu::~GameState_MainMenu()
	{
	}

	void GameState_MainMenu::update(float dt)
	{
		// handle input

		// mouse move select state

		// arrow keys select state

		// mouse click = enter = use selected state
	}

	void GameState_MainMenu::draw(sf::RenderTarget * target)
	{
		if (game_ && game_->window_)
			target->setView(*game_->guiView_);

		for (auto e : menuentries_)
			target->draw(e.first);
	}

	void GameState_MainMenu::onWindowSizeChanged(int w, int h)
	{
		if (menuentries_.size() == 0)
			return;

		// factor of the full height used for the menu
		float heightfactor = 0.7f;
		sf::Vector2u size(w, h*heightfactor);


		//calculate vertical space for each entry
		float entryheight = size.y / menuentries_.size();
		//float charactersize_old = menuentries_[0].first.getCharacterSize();
		
		// set new charactersize
		float charactersize = entryheight * 0.6f;
		for (auto& e : menuentries_)
		{
			e.first.setCharacterSize(charactersize);
			auto textSize = e.first.getLocalBounds();
			if (textSize.width > w)
				charactersize = charactersize * (0.9f*(float)w / textSize.width);
		}
		// character size changed because at least one entry exeeded the line, re-apply char size to all entries
		if (charactersize != entryheight * 0.6f)
		{
			for (auto& e : menuentries_)
				e.first.setCharacterSize(charactersize);
		}
			
		for (int i = 0; i < menuentries_.size(); i++)
		{
			//calc position
			auto& text = menuentries_[i].first;
			auto textSize = text.getLocalBounds();

			
			text.setPosition((size.x - textSize.width) / 2, h*(1.0f-heightfactor)/2 + entryheight * i + (entryheight-textSize.height)/2);
		}
	}

}