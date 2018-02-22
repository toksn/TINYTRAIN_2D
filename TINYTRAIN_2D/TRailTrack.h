#pragma once
#include "Entity.h"
#include "Spline_CatmullRom.h"
#include <memory>
#include <SFML\Graphics.hpp>

namespace tinytrain
{
	class TTrain;

	class TRailTrack : public tgf::Entity
	{
	public:
		TRailTrack();
		~TRailTrack();

		void append(const sf::Vector2f & a_ctrlPt);

		// Inherited via Entity
		virtual void draw(sf::RenderTarget * target) override;
		virtual void update(float deltaTime) override;

		// functions for moving trains along the track
		void addTrain(TTrain* a_train, float a_atDistance = 0.0f);
		void moveAndRotateOnRail(TTrain * train);
		//int getSegmentStartIndexAtDist(float a_dist, int indexHint = -1);


		float getSegmentLength();
		void setSegmentLength(float a_len);
		void addDrawnLinePoints(std::vector<sf::Vector2f> a_points, sf::Color a_color);

		// contains the length at each of the vertices
		std::vector<float> m_length;

		// array of trains actually driving on the track
		std::vector<TTrain*> m_trains;
		//sf::VertexArray m_trackspline;
		
		std::unique_ptr<tgf::math::Spline_CatmullRom> m_trackspline;
	private:
		float m_segLength;
	};
}