#pragma once
#include <SFML\Graphics.hpp>

class EntityManager;

class Entity
{
public:
	Entity(EntityManager* man);
	virtual ~Entity();
	virtual void draw(sf::RenderWindow& target) = 0;
	virtual void update(float deltaTime) = 0;

	bool destroyed = false;
	EntityManager* m_manager;

	// world pos
	sf::Transformable m_pos;

	// for drawing
	sf::Transformable* m_transformable;
	sf::Drawable* m_drawable;
};

