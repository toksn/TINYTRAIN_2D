//#include "Windows.h"
//#include <thread>
//#include <chrono>

#include "Game.h"

namespace tgf
{
	Game::Game(std::string game_name, int window_width, int window_height, int framerate_limit)
	{
		m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(window_width, window_height), game_name);
		m_window->setFramerateLimit(framerate_limit);

		m_frameClock = std::make_unique<sf::Clock>();
		m_bShowFPS = false;
	}


	Game::~Game()
	{
	}

	void Game::run()
	{
		while (m_window && m_window->isOpen())
		{
			sf::Event event;
			while (m_window->pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					m_window->close();

				if (event.type == sf::Event::Resized)
				{
					auto size = m_window->getSize();
					printf("\nwindow size changed: %i x %i", size.x, size.y);

					m_window->setView(sf::View(sf::FloatRect(0.0f, 0.0f, (float)size.x, (float)size.y)));
				}
			}

			// update the game
			update();
		}
	}

	const sf::Time g_timingBuffer = sf::microseconds(500);
	void Game::update()
	{
		// check for elapsed time and restart clock for next cycle
		sf::Time deltaTime = m_frameClock->restart();
		float curFPS = 1.0f / deltaTime.asSeconds();

		GameStateBase* currentState = peekState();
		if (currentState == nullptr)
			return;

		currentState->handleInput();
		currentState->update(deltaTime.asSeconds());

		m_window->clear(sf::Color::Black);
		currentState->draw(m_window.get());
		m_window->display();
		
		if (m_bShowFPS)
			printf("\nFPS: %f", 1.0f / m_renderTimer.asSeconds());

		if (m_desiredFrameTime != sf::Time::Zero)
		{
			// add render timer in seconds
			m_renderTimer += deltaTime;
			// do fixed render steps until theres less than desired timeframe left
			//if(m_renderTimer >= m_desiredFrameTime)
			{
				if (m_window)
				{
					m_window->clear();
					m_gameManager->draw(*m_window);

					//m_guiManager->draw(*m_window);
					m_window->display();
				}

				if (m_desiredFrameTime == sf::Time::Zero)
					m_renderTimer = sf::Time::Zero;
				else
				{
					while (m_renderTimer>m_desiredFrameTime)
						m_renderTimer -= m_desiredFrameTime;
				}
			}
			/*
			sf::Time elapsedFrameTime = m_frameClock->getElapsedTime() + m_renderTimer;
			if (elapsedFrameTime < m_desiredFrameTime)
			{
			// use sf::sleep for a more or less exact sleep because they use temporary high system precision
			sf::Time timeToSleep = m_desiredFrameTime - elapsedFrameTime;
			if (timeToSleep > g_timingBuffer)
			{
			sf::sleep(timeToSleep - g_timingBuffer);

			const sf::Time timeAfterSleep = m_frameClock->getElapsedTime() + m_renderTimer;
			const sf::Time actualTimeSlept = timeAfterSleep - elapsedFrameTime;
			printf("\nwanted to sleep: %dms\t\tactual time slept: %dms", (timeToSleep - g_timingBuffer).asMilliseconds(), actualTimeSlept.asMilliseconds());
			}

			// busy wait for the small rest of the frame (~1ms)
			while (m_frameClock->getElapsedTime() +m_renderTimer - elapsedFrameTime < timeToSleep)
			{
			std::this_thread::yield();
			}

			}*/
		}
	}

	void Game::handleGlobalInput()
	{
		// F10 to toggle FPS
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::F10))
			m_bShowFPS = !m_bShowFPS;
	}

	void Game::changeState(std::unique_ptr<GameStateBase> state)
	{
		if (m_states.empty() == false)
			m_states.pop();
		m_states.push(std::move(state));
	}

	GameStateBase * Game::peekState()
	{
		if (m_states.empty() == false)
			return m_states.top().get();
		return nullptr;
	}
		
	void Game::setMaxFPS(sf::Uint16 maxFPS)
	{
		if (maxFPS == 0)
			m_desiredFrameTime = sf::Time::Zero;
		else
		{
			m_maxFPS = maxFPS;
			//m_renderStep = sf::microseconds( 1.0f / m_maxFPS);
			m_desiredFrameTime = sf::microseconds(sf::Int64(1000000.0 / m_maxFPS));
			//m_desiredFrameTime = sf::microseconds(sf::Int64(1000000.0 / m_maxFPS));
		}
	}
}