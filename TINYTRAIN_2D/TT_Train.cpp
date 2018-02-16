#include "TT_Train.h"



TT_Train::TT_Train(EntityManager* man) : Entity(man)
{
	m_color_firstwagon = sf::Color::Yellow;
	m_color_wagons = sf::Color::White;

	m_wagonsize = sf::Vector2f(20.0f, 10.0f);
	m_wagongap = 5.0f;
	m_speed = m_wagonsize.x * 1.5f;


}


TT_Train::~TT_Train()
{
}

void TT_Train::initWagons(const unsigned int a_numberOfWagons)
{
	if (a_numberOfWagons < m_wagons.size())
	{
		while(a_numberOfWagons < m_wagons.size())
			m_wagons.pop_back();
	}
	else
	{
		
		while (a_numberOfWagons > m_wagons.size())
		{
			auto rect = sf::RectangleShape(m_wagonsize);
			rect.setFillColor(m_color_wagons);
			rect.setOrigin(m_wagonsize / 2.0f);
			m_wagons.push_back(rect);
		}
	}

	// set color for the leading wagon
	if (m_wagons.size() > 0)
		m_wagons[0].setFillColor(m_color_firstwagon);
}

/*
void TT_Train::setRailRoad(TT_RailRoad* a_railroad, float atDistance)
{
	m_railroad = a_railroad;
	if (atDistance >= 0.0f && atDistance < m_railroad->getLength())
		m_distance = atDistance;
}*/

void TT_Train::update(const float dt)
{
	// move the wagons by speed * dt on the railroad
	m_distance += dt * m_speed;
}

void TT_Train::draw(sf::RenderWindow & target)
{
	// draw them wagons
	for (int i = 0; i < m_wagons.size(); i++)
		target.draw(m_wagons[i]);
}
