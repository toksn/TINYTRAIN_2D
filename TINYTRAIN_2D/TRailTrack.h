#pragma once
#include "Entity.h"
#include <SFML\Graphics.hpp>

namespace tinytrain
{
	class TTrain;

	class TRailTrack : public tgf::Entity
	{
	public:
		TRailTrack();
		~TRailTrack();

		void recalcLength(unsigned int startindex = 0);
		float getLength();
		void append(const sf::Vertex& vertex);

		// Inherited via Entity
		virtual void draw(sf::RenderTarget * target) override;
		virtual void update(float deltaTime) override;

		// functions for moving trains along the track
		void addTrain(TTrain* a_train, float a_atDistance = 0.0f);
		void moveAndRotateOnRail(TTrain * train);
		void setPositionAndRotationFromRail(float a_dist, int index, sf::Transformable* obj);
		int getSegmentStartIndexAtDist(float a_dist, int indexHint = -1);

		// contains the length at each of the vertices
		std::vector<float> m_length;

		// array of trains actually driving on the track
		std::vector<TTrain*> m_trains;
		sf::VertexArray m_trackspline;
	};
}