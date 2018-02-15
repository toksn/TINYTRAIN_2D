#include "TT_Train.h"



TT_Train::TT_Train(EntityManager* man) : Entity(man)
{

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
			auto rect = sf::RectangleShape(sf::Vector2f(10.0f, 20.0f));
			rect.setFillColor(sf::Color::White);
			m_wagons.push_back(rect);
		}
	}

	// set color for the leading wagon
	if (m_wagons.size() > 0)
		m_wagons[0].setFillColor(sf::Color::Yellow);
}

void TT_Train::update(const float dt)
{
	// move the wagons by speed * dt on the railroad
	float dist = dt * m_speed;

	for (int i = 0; i < m_wagons.size(); i++)
	{
		m_wagons[i].setPosition();
	}
}

void TT_Train::draw(sf::RenderWindow & target)
{
	// draw them wagons
	for (int i = 0; i < m_wagons.size(); i++)
		target.draw(m_wagons[i]);
}
