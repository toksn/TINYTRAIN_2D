#include "Spline_CatmullRom.h"
#include "tinyc2.h"

namespace tgf
{
	namespace math
	{
		Spline_CatmullRom::Spline_CatmullRom()
		{
		}

		Spline_CatmullRom::~Spline_CatmullRom()
		{
		}

		void Spline_CatmullRom::onControlPointsAdded(size_t a_startindex)
		{
			if (m_controlPoints.getVertexCount() < 4)
			{
				return;
			}

			int new_control_point_index = m_controlPoints.getVertexCount() - 1;
			int pt = new_control_point_index - 2;
			for (int i = 0; i <= m_pointsPerSegment; i++)
			{
				float u = (float)i / (float)m_pointsPerSegment;

				appendSplinePoint(interpolate(u, m_controlPoints[pt - 1].position, m_controlPoints[pt].position, m_controlPoints[pt + 1].position, m_controlPoints[pt + 2].position));
			}
		}

		sf::Vector2f Spline_CatmullRom::interpolate(float u, sf::Vector2f pt0, sf::Vector2f pt1, sf::Vector2f pt2, sf::Vector2f pt3)
		{
			sf::Vector2f point;
			point.x = interpolate(u, pt0.x, pt1.x, pt2.x, pt3.x);
			point.y = interpolate(u, pt0.y, pt1.y, pt2.y, pt3.y);
			return point;
		}

		float Spline_CatmullRom::interpolate(float u, float pt0, float pt1, float pt2, float pt3)
		{
			float point = u*u*u*	((-1) * pt0 + 3 * pt1 - 3 * pt2 + pt3) / 2;
			point += u * u*(2 * pt0 - 5 * pt1 + 4 * pt2 - pt3) / 2;
			point += u * ((-1) * pt0 + pt2) / 2;
			point += pt1;

			return point;
		}
	}
}