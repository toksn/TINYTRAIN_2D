#include "GameState_Running.h"
#include <algorithm>

#include "TT_RailRoad.h"
#include "TT_Train.h"


GameState_Running::GameState_Running(tgf::Game * game)
{
	m_game = game;
	loadLevel();
}

GameState_Running::~GameState_Running()
{
}

void GameState_Running::update(float deltaTime)
{
	// update level
}

void GameState_Running::loadLevel()
{
	/**************************************************************************
	SIMPLE LEVEL CREATED BY CODE -- this is the minimum requirement for a level
	***************************************************************************
	// create train for the player
	TT_Train* playertrain = create<TT_Train>();

	// create a railroad for the train
	TT_RailRoad* rails = create<TT_RailRoad>();

	rails->append(sf::Vector2f(200.0f, 50.f));
	rails->append(sf::Vector2f(200.0f, 100.f));
	rails->append(sf::Vector2f(250.0f, 140.f));
	rails->append(sf::Vector2f(150.0f, 180.f));
	rails->append(sf::Vector2f(130.0f, 70.f));

	sf::Vector2f lastPos(130.0f, 70.f);
	for (size_t i = 0; i < 50; i++)
	{
		lastPos.x += rand() % 200 - 100;
		lastPos.y += rand() % 200 - 100;
		
		if (lastPos.x < 0)
			lastPos.x = 0;
		else if (lastPos.y > 600)
			lastPos.x = 590;

		if (lastPos.y < 0)
			lastPos.y = 0;
		else if (lastPos.y > 400)
			lastPos.y = 400;

		rails->append(lastPos);
	}

	rails->addTrain(playertrain);
	playertrain->initWagons(3);

	// create obstacles for the games to be lost
	//create<TT_Obstacle>();

	// create target zone for the game to be won
	//create<TT_TargetZone>();
	/************************************************************************/

	// TODO:
	// passengers to pick up
}

void GameState_Running::draw(sf::RenderTarget * target)
{
}

void GameState_Running::handleInput()
{
}
