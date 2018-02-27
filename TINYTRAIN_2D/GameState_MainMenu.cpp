#include "GameState_MainMenu.h"
#include "GameState_Running.h"
#include "Game.h"


namespace tinytrain
{
	GameState_MainMenu::GameState_MainMenu(tgf::Game* game)
	{
		game_ = game;

		font_.loadFromFile("data/fonts/pixantiqua.ttf");

		menuentries_.push_back(menuEntry(sf::Text("start", font_),								std::bind(&GameState_MainMenu::onStart, this)));
		menuentries_.push_back(menuEntry(sf::Text("options", font_),							std::bind(&GameState_MainMenu::onStart, this)));
		menuentries_.push_back(menuEntry(sf::Text("blabla12354789+$%\"(§", font_),				std::bind(&GameState_MainMenu::onStart, this)));
		menuentries_.push_back(menuEntry(sf::Text("thelazybrownfox", font_),					std::bind(&GameState_MainMenu::onStart, this)));
		menuentries_.push_back(menuEntry(sf::Text("jumpsovertheidontknowwhatsmissing", font_),	std::bind(&GameState_MainMenu::onStart, this))); 
		menuentries_.push_back(menuEntry(sf::Text("quit", font_),								std::bind(&GameState_MainMenu::onQuit, this)));
		
		selection_ = 0;

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
		for (int i = 0; i < menuentries_.size(); i++)
		{
			if(i == selection_)
				menuentries_[i].text_.setFillColor(sf::Color::Red);
			else
				menuentries_[i].text_.setFillColor(sf::Color::White);
		}
	}

	void GameState_MainMenu::draw(sf::RenderTarget * target)
	{
		if (game_ && game_->window_)
			target->setView(*game_->guiView_);

		for (auto e : menuentries_)
			target->draw(e.text_);
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
			e.text_.setCharacterSize(charactersize);
			e.localbounds_ = e.text_.getLocalBounds();
			if (e.localbounds_.width > w)
				charactersize = charactersize * (0.9f*(float)w / e.localbounds_.width);
		}
		// character size changed because at least one entry exeeded the line, re-apply char size to all entries
		if (charactersize != entryheight * 0.6f)
		{
			for (auto& e : menuentries_)
			{
				e.text_.setCharacterSize(charactersize);
				e.localbounds_ = e.text_.getLocalBounds();
			}				
		}
			
		for (int i = 0; i < menuentries_.size(); i++)
		{
			//calc position
			auto& text = menuentries_[i].text_;
			auto textSize = menuentries_[i].localbounds_;

			menuentries_[i].text_.setPosition((size.x - textSize.width) / 2, h*(1.0f-heightfactor)/2 + entryheight * i + (entryheight-textSize.height)/2);
			menuentries_[i].globalbounds_ = menuentries_[i].text_.getGlobalBounds();
		}
	}

	void GameState_MainMenu::handleInput(sf::Event & e)
	{
		tgf::GameStateBase::handleInput(e);

		int i = 0;

		// handle input for menu itself
		switch (e.type)
		{

		case sf::Event::KeyPressed:
			// arrow keys to select state
			if (menuentries_.size() > 1)
			{
				if (e.key.code == sf::Keyboard::Key::Up)
					selection_ = (selection_ - 1 + menuentries_.size()) % menuentries_.size();
				else if (e.key.code == sf::Keyboard::Key::Down)
					selection_ = (selection_ + 1) % menuentries_.size();
				else if (e.key.code == sf::Keyboard::Key::Return)
					executeSelectedEntry();
			}
			break;
		case sf::Event::MouseMoved:
			// mouse move to select state
			i = getEntryIndexAtPosition(e.mouseMove.x, e.mouseMove.y);
			if (i != -1)
				selection_ = i;
			break;
		case sf::Event::MouseButtonPressed:
			// left mouse click = use selected state
			if (e.mouseButton.button == sf::Mouse::Left)
			{
				i = getEntryIndexAtPosition(e.mouseButton.x, e.mouseButton.y);
				if (i != -1)
					executeSelectedEntry();
			}
			break;
		default:
			break;
		}
	}
	void GameState_MainMenu::executeSelectedEntry()
	{
		if (menuentries_.size() > 0 && menuentries_[selection_].func_)
			menuentries_[selection_].func_();
	}

	int GameState_MainMenu::getEntryIndexAtPosition(int x, int y)
	{
		for (int i = 0; i < menuentries_.size(); i++)
			if (menuentries_[i].globalbounds_.contains(x, y))
				return i;

		return -1;
	}

	void GameState_MainMenu::onStart()
	{
		if (game_)
			game_->changeState(std::make_unique<tinytrain::GameState_Running>(game_));
	}

	void GameState_MainMenu::onQuit()
	{
		if (game_ && game_->window_)
			game_->window_->close();
	}

}