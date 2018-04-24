//#define TEST_HEADER
#ifdef TEST_HEADER
#include "GameStateBase.h"
#else

//#include "Windows.h"
//#include <thread>
//#include <chrono>
//#include "TextureAtlas.h"
#include "Game.h"
#include "GameStateBase.h"

namespace tgf
{
	Game::Game(std::string game_name, int window_width, int window_height, int framerate_limit)
	{
		window_ = std::make_unique<sf::RenderWindow>(sf::VideoMode(window_width, window_height), game_name, sf::Style::Default);
		window_->setFramerateLimit(framerate_limit);

		guiView_ = std::make_unique<sf::View>(sf::FloatRect(0.0f, 0.0f, window_width, window_height));

		frameClock_ = std::make_unique<sf::Clock>();
		bShowFPS_ = false;
	}


	Game::~Game()
	{
	}

	void Game::run()
	{
		while (window_ && window_->isOpen())
		{
			removedStates_.clear();

			handleGlobalInput();

			// update the game
			update();
		}
	}

	const sf::Time g_timingBuffer = sf::microseconds(500);
	void Game::update()
	{
		// check for elapsed time and restart clock for next cycle
		sf::Time deltaTime = frameClock_->restart();
		float curFPS = 1.0f / deltaTime.asSeconds();

		GameStateBase* currentState = peekState();
		if (currentState == nullptr)
		{
			// no gamestate available, close game
			window_->close();
			return;
		}

		currentState->update(deltaTime.asSeconds());

		window_->clear(sf::Color::Black);
		currentState->draw(window_.get());
		window_->display();
		
		if (bShowFPS_)
			printf("\nFPS: %f", 1.0f / renderTimer_.asSeconds());

		if (desiredFrameTime_ != sf::Time::Zero)
		{
			// add render timer in seconds
			renderTimer_ += deltaTime;
			// do fixed render steps until theres less than desired timeframe left
			while (renderTimer_>desiredFrameTime_)
				renderTimer_ -= desiredFrameTime_;
			
			/*
			sf::Time elapsedFrameTime = frameClock_->getElapsedTime() + renderTimer_;
			if (elapsedFrameTime < desiredFrameTime_)
			{
			// use sf::sleep for a more or less exact sleep because they use temporary high system precision
			sf::Time timeToSleep = desiredFrameTime_ - elapsedFrameTime;
			if (timeToSleep > g_timingBuffer)
			{
			sf::sleep(timeToSleep - g_timingBuffer);

			const sf::Time timeAfterSleep = frameClock_->getElapsedTime() + renderTimer_;
			const sf::Time actualTimeSlept = timeAfterSleep - elapsedFrameTime;
			printf("\nwanted to sleep: %dms\t\tactual time slept: %dms", (timeToSleep - g_timingBuffer).asMilliseconds(), actualTimeSlept.asMilliseconds());
			}

			// busy wait for the small rest of the frame (~1ms)
			while (frameClock_->getElapsedTime() +renderTimer_ - elapsedFrameTime < timeToSleep)
			{
			std::this_thread::yield();
			}

			}*/
		}
	}

	void Game::handleGlobalInput()
	{
		sf::Event event;
		while (window_->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window_->close();
			else if (event.type == sf::Event::Resized)
			{
				auto size = window_->getSize();
				printf("\nwindow size changed: %i x %i\n", size.x, size.y);
				guiView_->reset(sf::FloatRect(0.0f, 0.0f, (float)size.x, (float)size.y));

				// inform all current gamestates
				for (auto& state : states_)
					state->onWindowSizeChanged(size.x, size.y);
			}
			else if(states_.size())
				states_.back().get()->handleInput(event);
		}

		// F10 to toggle FPS
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F10))
			bShowFPS_ = !bShowFPS_;
	}

	void Game::changeState(std::unique_ptr<GameStateBase> state)
	{
		popState();
		states_.push_back(std::move(state));
	}

	void Game::pushState(std::unique_ptr<GameStateBase> state)
	{
		if(state)
			states_.push_back(std::move(state));
	}

	void Game::popState()
	{
		if (states_.empty() == false)
		{
			// dont remove it directly because the states_ may remove themselves from the vector and thus get killed while in execution
			removedStates_.push_back(std::move(states_.back()));
			states_.pop_back();
		}
	}

	GameStateBase * Game::peekState()
	{
		if (states_.empty() == false)
			return states_.back().get();
		return nullptr;
	}

	bool Game::loadFont(std::string font_file_path)
	{
		if (font_ == nullptr)
			font_ = std::make_unique<sf::Font>();

		return font_->loadFromFile(font_file_path);
	}

	bool Game::loadTextureAtlas(std::string texture_file_path)
	{
		if (texture_atlas_ == nullptr)
			texture_atlas_ = std::make_unique<utilities::TextureAtlas>();

		return texture_atlas_->init(texture_file_path);
	}

	utilities::TextureAtlas* Game::getTextureAtlas()
	{
		return texture_atlas_.get();
	}
		
	void Game::setMaxFPS(sf::Uint16 maxFPS)
	{
		if (maxFPS == 0)
			desiredFrameTime_ = sf::Time::Zero;
		else
		{
			maxFPS_ = maxFPS;
			//renderStep_ = sf::microseconds( 1.0f / maxFPS_);
			desiredFrameTime_ = sf::microseconds(sf::Int64(1000000.0 / maxFPS_));
			//desiredFrameTime_ = sf::microseconds(sf::Int64(1000000.0 / maxFPS_));
		}
	}
}
#endif // TEST_HEADER