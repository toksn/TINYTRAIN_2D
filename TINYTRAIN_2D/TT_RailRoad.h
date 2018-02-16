#pragma once
#include <SFML\Graphics.hpp>
class TT_Train;

class TT_RailRoad : public sf::VertexArray
{
public:
	TT_RailRoad();
	~TT_RailRoad();

	void recalcLength(unsigned int startindex = 0);
	void append(const sf::Vertex& vertex);

	float getLength();

	void moveAndRotateOnRail(float a_dist, TT_Train * train);

	void setPositionAndRotationFromRail(float a_dist, int index, sf::Transformable* obj);
	
	sf::Vector2f getPositionOnRail(float a_dist, int index = -1);
	int getSegmentStartIndexAtDist(float a_dist, int indexHint = -1);

	// contains the length at each of the vertices
	std::vector<float> m_length;
};