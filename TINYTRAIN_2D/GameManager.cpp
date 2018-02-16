#include "GameManager.h"
#include <algorithm>

#include "TT_RailRoad.h"
#include "TT_Train.h"



GameManager::GameManager() : EntityManager()
{
	m_physicsTimer = 0.0f;
}


GameManager::~GameManager()
{
}

void GameManager::update(float deltaTime)
{
	// add physics timer in seconds
	m_physicsTimer += deltaTime;
	// do fixed physics steps until theres less than 16.666ms left
	while (m_physicsTimer >= PHYSICS_STEP)
	{
		//m_world->Step(PHYSICS_STEP, 3, 4);

		for (auto& e : m_entities) e->update(PHYSICS_STEP);

		m_physicsTimer -= PHYSICS_STEP;
	}

	
}

void GameManager::loadLevel()
{
	/**************************************************************************
	SIMPLE LEVEL CREATED BY CODE -- this is the minimum requirement for a level
	***************************************************************************/
	// create train for the player
	TT_Train* playertrain = create<TT_Train>();

	// create a railroad for the train
	auto rails = new TT_RailRoad();

	rails->append(sf::Vector2f(200.0f, 50.f));
	rails->append(sf::Vector2f(200.0f, 100.f));
	rails->append(sf::Vector2f(250.0f, 140.f));
	rails->append(sf::Vector2f(150.0f, 180.f));
	rails->append(sf::Vector2f(130.0f, 70.f));
	playertrain->setRailRoad(rails);
	playertrain->initWagons(3);

	// create obstacles for the games to be lost
	//create<TT_Obstacle>();

	// create target zone for the game to be won
	//create<TT_TargetZone>();
	/************************************************************************/

	// TODO:
	// passengers to pick up
}
