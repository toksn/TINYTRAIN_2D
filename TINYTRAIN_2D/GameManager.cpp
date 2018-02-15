#include "GameManager.h"
#include <algorithm>



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
	***************************************************************************
	// create train for the player
	create<TT_Train>();

	// create a railroad for the train
	create<TT_RailRoad>();

	// create obstacles for the games to be lost
	create<TT_Obstacle>();

	// create target zone for the game to be won
	create<TT_TargetZone>();
	/************************************************************************/

	// TODO:
	// passengers to pick up
}