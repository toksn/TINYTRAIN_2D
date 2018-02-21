#include "Spline.h"
#include "tinyc2.h"

namespace tgf
{
	namespace math
	{
		Spline::Spline()
		{
			// default railtrack draw type
			m_controlPoints.setPrimitiveType(sf::PrimitiveType::LinesStrip);
			m_splinePoints.setPrimitiveType(sf::PrimitiveType::LinesStrip);

			m_pointsPerSegment = 20;
			m_drawControlPoints = true;

			m_color = sf::Color::Red;
			m_color_controlpts = sf::Color::White;
		}
		
		Spline::~Spline()
		{
		}

		void Spline::draw(sf::RenderTarget * target)
		{
			if (m_drawControlPoints)
				target->draw(m_controlPoints);

			target->draw(m_splinePoints);
		}

		void Spline::update(float deltaTime)
		{
			// ?
		}

		sf::Vector2f Spline::getLocationAtTime(float a_time)
		{
			int index = -1;
			return getLocationAtTime(a_time, index);
		}


		sf::Vector2f Spline::getLocationAtTime(float a_time, int& indexHint )
		{
			if (m_splinePoints.getVertexCount() == 0)
				return sf::Vector2f();

			c2v pos;
			if (a_time == 1.0f)
				pos = { m_splinePoints[m_splinePoints.getVertexCount() - 1].position.x, m_splinePoints[m_splinePoints.getVertexCount() - 1].position.y };
			else if (a_time == 0.0f)
				pos = { m_splinePoints[0].position.x, m_splinePoints[0].position.y };
			else
			{
				float a_dist = getLength()*a_time;
				indexHint = getSegmentStartIndexAtDist(a_dist, indexHint);
				if (indexHint < 0 || indexHint >= m_splinePoints.getVertexCount())
					return sf::Vector2f();

				// segment
				c2v start = { m_splinePoints[indexHint].position.x, m_splinePoints[indexHint].position.y };
				c2v end = { m_splinePoints[indexHint + 1].position.x, m_splinePoints[indexHint + 1].position.y };

				// calc rest dist on the segment
				a_dist -= m_splinePointsLengths[indexHint];

				// make it 0-1 range based on the segment
				a_dist = a_dist / c2Len(c2Sub(end, start));
				pos = c2Lerp(start, end, a_dist);
			}

			return sf::Vector2f(pos.x, pos.y);
		}
		float Spline::getDirectionAngleAtTime(float a_time, bool a_in_radiant)
		{
			int index = -1;
			return getDirectionAngleAtTime(a_time, index, a_in_radiant);
		}
		float Spline::getDirectionAngleAtTime(float a_time, int& indexHint, bool a_in_radiant)
		{
			// no direction calc possible when not enough points
			if (m_splinePoints.getVertexCount() < 2)
				return 0.0f;


			c2v start, end, seg;
			if (a_time == 1.0f)
			{
				start = { m_splinePoints[m_splinePoints.getVertexCount() - 2].position.x, m_splinePoints[m_splinePoints.getVertexCount() - 2].position.y };
				end = { m_splinePoints[m_splinePoints.getVertexCount() - 1].position.x, m_splinePoints[m_splinePoints.getVertexCount() - 1].position.y };
			}
			else if (a_time == 0.0f)
			{
				start = { m_splinePoints[0].position.x, m_splinePoints[0].position.y };
				end = { m_splinePoints[1].position.x, m_splinePoints[1].position.y };
			}
			else
			{
				indexHint = getSegmentStartIndexAtDist(getLength()*a_time, indexHint);
				if (indexHint < 0 || indexHint >= m_splinePoints.getVertexCount())
					return 0.0f;

				start = { m_splinePoints[indexHint].position.x, m_splinePoints[indexHint].position.y };
				end = { m_splinePoints[indexHint + 1].position.x, m_splinePoints[indexHint + 1].position.y };
			}

			seg = c2Sub(end, start);
			// return in radiant or degree
			return a_in_radiant ? atan2(seg.y, seg.x) : atan2(seg.y, seg.x)*RAD_TO_DEG;
		}
		float Spline::getLength()
		{
			if (m_splinePointsLengths.size() != m_splinePoints.getVertexCount())
				recalcLength();

			if (m_splinePoints.getVertexCount() == 0)
				return 0.0f;

			return m_splinePointsLengths.back();
		}

		void Spline::recalcLength(unsigned int startindex)
		{
			size_t size = m_splinePoints.getVertexCount();
			m_splinePointsLengths.resize(size);

			// first point always has length zero
			if (startindex == 0)
			{
				m_splinePointsLengths[startindex] = 0;
				startindex++;
			}

			// we can assume here that the startindex is > 0 because of the above increment
			for (int i = startindex; i < size; i++)
				m_splinePointsLengths[i] = m_splinePointsLengths[i - 1] + c2Len(c2Sub(c2v{ m_splinePoints[i].position.x , m_splinePoints[i].position.y }, c2v{ m_splinePoints[i - 1].position.x , m_splinePoints[i - 1].position.y }));
		}


		float Spline::getLengthAtTime(float a_time)
		{
			return getLength() * a_time;
		}
		
		void Spline::appendControlPoint(sf::Vector2f a_pt)
		{
			auto size_before = m_controlPoints.getVertexCount();
			m_controlPoints.append(sf::Vertex(a_pt, m_color_controlpts));

			onControlPointsAdded(size_before);
		}

		bool Spline::getLastControlPoint(sf::Vector2f & a_pt)
		{
			if(m_controlPoints.getVertexCount() == 0)
				return false;

			 a_pt = m_controlPoints[m_controlPoints.getVertexCount() - 1].position;
			 return true;
		}

		bool Spline::getLastControlPointSegment(sf::Vector2f & a_start, sf::Vector2f & a_end)
		{
			if (m_controlPoints.getVertexCount() < 2)
				return false;

			a_start = m_controlPoints[m_controlPoints.getVertexCount() - 2].position;
			a_end	= m_controlPoints[m_controlPoints.getVertexCount() - 1].position;
			return true;
		}

		void Spline::appendSplinePoint(sf::Vector2f a_pt)
		{
			m_splinePoints.append(sf::Vertex(a_pt, m_color));

			recalcLength(m_splinePoints.getVertexCount() - 1);	
		}

		int Spline::getSegmentStartIndexAtTime(float a_time, int indexHint)
		{
			return getSegmentStartIndexAtDist(a_time / getLength(), indexHint);
		}

		// return -1 in case of an error
		int Spline::getSegmentStartIndexAtDist(float a_dist, int indexHint)
		{
			int i = 0;

			size_t size = m_splinePoints.getVertexCount();

			if (size < 0)
				return -1;
			else if (size > 1)
			{
				float len = getLength();

				// take indexHint or make an index guess
				if (indexHint >= 0 && indexHint < size - 1)
					i = indexHint;
				else
					i = c2Min(((float)(size - 1) * a_dist / len), size - 2);

				// keep index range
				while (i >= 0 && i < size - 1)
				{
					if (m_splinePointsLengths[i] <= a_dist)
					{
						// found correct segment
						if (m_splinePointsLengths[i + 1] > a_dist)
							break;
						else
							i++;
					}
					else
						i--;
				}
			}

			return i;
		}
	}
}