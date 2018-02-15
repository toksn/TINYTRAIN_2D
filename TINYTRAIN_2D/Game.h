#pragma once
#include <memory>
#include <SFML/Graphics.hpp>
#include "GameManager.h"
#include "GuiManager.h"

class Game
{
public:
	Game();
	~Game();

	void update();
	void setMaxFPS(sf::Uint16 maxFPS);
	void run();
	void restart();

	bool m_bShowFPS;
	sf::RenderWindow* m_window;

private:
	std::unique_ptr<sf::Clock> m_frameClock;
	std::unique_ptr<GameManager> m_gameManager;
	std::unique_ptr<GuiManager> m_guiManager;
	sf::Uint16	m_maxFPS;
	//sf::Time	m_renderStep;
	sf::Time	m_renderTimer;
	sf::Time	m_desiredFrameTime;
};

