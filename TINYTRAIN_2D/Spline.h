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

			// colorizing
			sf::Color getColor();
			void setColor(sf::Color a_color, bool recolor_existing = true);
			sf::Color getColor_controlPts();
			void setColor_controlPts(sf::Color a_color, bool recolor_existing = true);


			sf::VertexArray controlPoints_;
			sf::VertexArray splinePoints_;

			bool drawControlPoints_;
		protected:
			// Inherited via Entity
			virtual void onDraw(sf::RenderTarget * target) override;
			virtual void onUpdate(float deltaTime) override;

			int getSegmentStartIndexAtTime(float a_time, int indexHint = -1);
			int getSegmentStartIndexAtDist(float a_dist, int indexHint = -1);

			// OVERRIDE THIS FOR A NEW SPLINE CLASS
			virtual void onControlPointsAdded(int a_startindex) = 0;

			void appendSplinePoint(sf::Vector2f a_pt);

			std::vector<float> splinePointsLengths_;
			int pointsPerSegment_;

			// colors
			sf::Color colorControlPts_;
			sf::Color color_;
		};
	}
}