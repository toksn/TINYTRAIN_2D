#pragma once
#include <SFML\Graphics.hpp>

class TT_RailRoad : public sf::VertexArray
{
public:
	TT_RailRoad();
	~TT_RailRoad();

	void recalcLength(unsigned int startindex = 0);
	void append(const sf::Vertex& vertex);

	float getLength();
	sf::Vector2f getPositionOnRail(float a_dist);

	// contains the length at each of the vertices
	std::vector<float> m_length;
};