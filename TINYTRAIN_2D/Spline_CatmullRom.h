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

		protected:
			// Inherited via Spline
			virtual void onControlPointsAdded(int a_startindex = 0) override;

			sf::Vector2f interpolateUniform(float u, sf::Vector2f pt0, sf::Vector2f pt1, sf::Vector2f pt2, sf::Vector2f pt3);
			sf::Vector2f interpolate(float u, sf::Vector2f pt0, sf::Vector2f pt1, sf::Vector2f pt2, sf::Vector2f pt3, float* time);
		};
	}
}

