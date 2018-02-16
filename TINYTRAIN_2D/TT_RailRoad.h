#pragma once
#include "Entity.h"
#include <SFML\Graphics.hpp>
class TT_Train;

class TT_RailRoad : public Entity
{
public:
	TT_RailRoad(EntityManager* man);
	~TT_RailRoad();

	void recalcLength(unsigned int startindex = 0);
	float getLength();
	void append(const sf::Vertex& vertex);

	// Inherited via Entity
	virtual void draw(sf::RenderWindow & target) override;
	virtual void update(float deltaTime) override;
	
	// functions for moving trains along the track
	void addTrain(TT_Train* a_train, float a_atDistance = 0.0f);
	void moveAndRotateOnRail(TT_Train * train);
	void setPositionAndRotationFromRail(float a_dist, int index, sf::Transformable* obj);
	int getSegmentStartIndexAtDist(float a_dist, int indexHint = -1);
	
	// contains the length at each of the vertices
	std::vector<float> m_length;

	// array of trains actually driving on the track
	std::vector<TT_Train*> m_trains;
	sf::VertexArray m_trackspline;
};