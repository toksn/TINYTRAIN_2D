#pragma once
#include "Spline.h"

namespace tgf
{
	namespace math
	{
		class Spline_CatmullRom : public Spline
		{
		public:
			// catmull rom type
			// see http://www.cemyuksel.com/research/catmullroparam_/catmullrom.pdf for more info
			enum class CatmullRomType
			{
				Uniform,		// uniform is faster, can introduce self intersections
				Chordal,		// chordal is more curvy, less intersections
				Centripetal		// centripetal in between, no intersections
			};

		
			Spline_CatmullRom();
			~Spline_CatmullRom();

			CatmullRomType type_;

			// a catmull rom spline needs 4 points (pt0, pt1, pt2, pt3) to create a segment from pt1 to pt2. 
			// therefore, naturally the first and last control point are never hit by the spline.
			// To interpolate all given control points, the first and last control points have to be used twice in calculation of the spline.
			//
			// Note: This leads to changes in the last two spline segments when a new control point is appended!
			bool interpolateControlPointEnds_;

			// normals
			std::vector<sf::Vector2f> normals_;
			bool calcNormals_;
			bool drawNormals_;

		protected:
			// Inherited via Spline
			virtual void onControlPointsAdded(int a_startindex = 0) override;
			virtual void onDraw(sf::RenderTarget* target) override;

			sf::Vector2f interpolateUniform(float u, sf::Vector2f pt0, sf::Vector2f pt1, sf::Vector2f pt2, sf::Vector2f pt3);
			sf::Vector2f interpolate(float u, sf::Vector2f pt0, sf::Vector2f pt1, sf::Vector2f pt2, sf::Vector2f pt3, float* time);
			sf::Vector2f tangent(float u, sf::Vector2f pt0, sf::Vector2f pt1, sf::Vector2f pt2, sf::Vector2f pt3, float * time);
		};
	}
}

