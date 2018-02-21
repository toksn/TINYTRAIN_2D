#pragma once
#include "Entity.h"

namespace tgf
{
	namespace math
	{
		class Spline : public Entity
		{
		public:
			Spline();
			~Spline();

			// Inherited via Entity
			virtual void draw(sf::RenderTarget * target) override;
			virtual void update(float deltaTime) override;

			sf::Vector2f getLocationAtTime(float a_time);
			sf::Vector2f getLocationAtTime(float a_time, int& hintindex);
			float getDirectionAngleAtTime(float a_time, bool a_in_radiant = true );
			float getDirectionAngleAtTime(float a_time, int& hintindex, bool a_in_radiant = true );

			float getLength();
			void recalcLength(unsigned int startindex = 0);
			float getLengthAtTime(float a_time);

			void appendControlPoint(sf::Vector2f a_pt);

			bool getLastControlPoint(sf::Vector2f& a_pt);
			bool getLastControlPointSegment(sf::Vector2f& a_start, sf::Vector2f& a_end);

			sf::Color m_color_controlpts;
			sf::Color m_color;

		protected:
			int getSegmentStartIndexAtTime(float a_time, int indexHint = -1);
			int getSegmentStartIndexAtDist(float a_dist, int indexHint = -1);

			// OVERRIDE THIS FOR A NEW SPLINE
			virtual void onControlPointsAdded(int a_startindex) = 0;

			void appendSplinePoint(sf::Vector2f a_pt);
			
			bool m_drawControlPoints;

			// controlpoints
			sf::VertexArray m_controlPoints;
			// spline
			sf::VertexArray m_splinePoints;
			std::vector<float> m_splinePointsLengths;
			int m_pointsPerSegment;
		};
	}
}

