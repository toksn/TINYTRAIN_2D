#pragma once
#include "Spline.h"

namespace tgf
{
	namespace math
	{
		class Spline_CatmullRom : public Spline
		{
		public:
			Spline_CatmullRom();
			~Spline_CatmullRom();

		protected:
			// Inherited via Spline
			virtual void onControlPointsAdded(int a_startindex = 0) override;

			virtual sf::Vector2f interpolate(float u, sf::Vector2f pt0, sf::Vector2f pt1, sf::Vector2f pt2, sf::Vector2f pt3);
			float interpolate(float u, float pt0, float pt1, float pt2, float pt3);
		};
	}
}

