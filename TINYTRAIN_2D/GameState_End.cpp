#include "GameState_End.h"
#include "GameState_Running.h"
#include "Game.h"


namespace tinytrain
{
	GameState_End::GameState_End(tgf::Game* game)
	{
		game_ = game;
		
		menu_ = std::make_unique<tgf::gui::TextMenu>();

		if (menu_ && game_)
		{
			menu_->maxEntryHeight_ = 70;

			menu_->appendItem(sf::Text("play again", *game_->font_), std::bind(&GameState_End::onRestart, this));
			menu_->appendItem(sf::Text("next level", *game_->font_), nullptr);
			menu_->appendItem(sf::Text("mainmenu", *game_->font_), std::bind(&GameState_End::onQuitToMainMenu, this));

			bindEventCallback(sf::Event::EventType::MouseMoved, menu_.get(), &tgf::gui::TextMenu::onMouseMove);
			bindEventCallback(sf::Event::EventType::MouseButtonPressed, menu_.get(), &tgf::gui::TextMenu::onMousePressed);
			bindEventCallback(sf::Event::EventType::KeyPressed, menu_.get(), &tgf::gui::TextMenu::onKeyPressed);

			if (game && game->window_)
			{
				onWindowSizeChanged(game->window_->getSize().x, game->window_->getSize().y);
			}
		}

		pause_grey_.setFillColor(sf::Color(0, 0, 0, 200));
	}

	GameState_End::~GameState_End()
	{
	}

	void GameState_End::update(float dt)
	{
		if(menu_)
			menu_->update(dt);
	}

	void GameState_End::draw(sf::RenderTarget * target)
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

		target->draw(*endtext_);
	}

	void GameState_End::onWindowSizeChanged(int w, int h)
	{
		if (menu_)
		{
			// factor of the full height and width used for the menu
			float heightfactor = 0.3f;
			float widthfactor = 0.9f;

			// position in the lower third of the screen
			sf::FloatRect size(w*(1 - widthfactor) / 2, h * (0.9f - heightfactor), w*widthfactor, h * heightfactor);
			menu_->setArea(size);
		}

		if (endtext_)
		{
			endtext_->setCharacterSize(0.1f * h);
			auto bb = endtext_->getGlobalBounds();
			if (bb.width > w)
			{
				endtext_->setCharacterSize(0.1f*h * w / bb.width);
				bb = endtext_->getGlobalBounds();
			}

			endtext_->setPosition((w - bb.width) / 2.0f, 0.3f * h);
		}		

		pause_grey_.setSize(sf::Vector2f(w, h));
	}

	void GameState_End::onRestart()
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
		if (game_)
			game_->popState();
	}

	void GameState_End::onQuitToMainMenu()
	{
		if (game_)
		{
			// remove pause and running state
			// todo: remove states until mainmenu
			game_->popState();
			game_->popState();
		}
	}

	void GameState_End::setEndText(std::string a_text, sf::Color a_color)
	{
		if (game_)
		{
			if (endtext_)
				endtext_->setString(sf::String(a_text));
			else
				endtext_ = std::make_unique<sf::Text>(sf::String(a_text), *game_->font_);

			endtext_->setFillColor(a_color);

			if (game_->window_)
			{
				auto size = game_->window_->getSize();
				onWindowSizeChanged(size.x, size.y);
			}
		}		
	}

}