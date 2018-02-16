#pragma once
#include <SFML\Graphics.hpp>
#include "Entity.h"
//#include "TT_RailRoad.h"

class TT_Train : public Entity
{
public:
	TT_Train(EntityManager* man);
	~TT_Train();

	void initWagons(const unsigned int a_numberOfWagons);

	// Inherited via Entity
	virtual void draw(sf::RenderWindow & target) override;
	virtual void update(const float dt) override;
	
	// wagon stats
	float m_speed;
	sf::Vector2f m_wagonsize;
	float m_wagongap;
	sf::Color m_color_firstwagon;
	sf::Color m_color_wagons;

	std::vector<sf::RectangleShape> m_wagons;
	//TT_RailRoad* m_railroad;

	// distance already travelled by the first wagon
	float m_distance;
	
};

