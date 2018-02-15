#pragma once
#include <SFML\Graphics.hpp>
#include "Entity.h"

class TT_Train : public Entity
{
public:
	TT_Train(EntityManager* man);
	~TT_Train();

	// Inherited via Entity
	virtual void draw(sf::RenderWindow & target) override;
	virtual void update(const float dt) override;

	void initWagons(const unsigned int a_numberOfWagons);

	float m_speed;
	std::vector<sf::RectangleShape> m_wagons;
	sf::VertexArray m_railroad;	
};

