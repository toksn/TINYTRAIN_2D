#include "EntityManager.h"
#include <algorithm>

EntityManager::EntityManager()
{
}


EntityManager::~EntityManager()
{
}

void EntityManager::draw(sf::RenderWindow& target)
{
	for (auto& e : m_entities)
	{
		e->draw(target);
	}
}

// todo: into camera
constexpr float oneMeterInPixel = 20.0f;
constexpr float onePixelInMeter = 1.0f/ oneMeterInPixel;
sf::Transformable EntityManager::convertToScreen(sf::Transformable worldPos)
{
	sf::Transformable trans = worldPos;

	return worldPos;
	//return sf::Vector2f(worldPos.x * oneMeterInPixel, worldPos.y * oneMeterInPixel);
}
sf::Vector2f EntityManager::convertPosToWorld(sf::Vector2f screenPos)
{
	return sf::Vector2f(screenPos.x * onePixelInMeter, screenPos.y * onePixelInMeter);
}

float EntityManager::convertToScreen(float worldSize)
{
	return worldSize*oneMeterInPixel;
}
float EntityManager::convertToWorld(float screenSize)
{
	return screenSize*onePixelInMeter;
}

void EntityManager::refresh()
{
	// delete raw pointers when marked as destroyed
	for (auto& pair : m_groupedEntities)
	{
		auto& vec(pair.second);
		vec.erase(std::remove_if(std::begin(vec), std::end(vec), [](auto ptr) {return ptr->destroyed; }), std::end(vec));
	}

	// delete unique pointer + element when marked as destroyed
	m_entities.erase(std::remove_if(std::begin(m_entities), std::end(m_entities), [](const auto& uptr) {return uptr->destroyed; }), std::end(m_entities));
}

void EntityManager::clear()
{
	m_entities.clear();
	m_groupedEntities.clear();
}