#include "Spline.h"
#include "tinyc2.h"

namespace tgf
{
	namespace math
	{
		Spline::Spline()
		{
			// default railtrack draw type
			controlPoints_.setPrimitiveType(sf::PrimitiveType::LinesStrip);
			splinePoints_.setPrimitiveType(sf::PrimitiveType::LinesStrip);

			pointsPerSegment_ = 20;
			drawControlPoints_ = true;

			color_ = sf::Color::Red;
			colorControlPts_ = sf::Color::White;
		}
		
		Spline::~Spline()
		{
		}

		void Spline::draw(sf::RenderTarget * target)
		{
			if (drawControlPoints_)
				target->draw(controlPoints_);

			target->draw(splinePoints_);
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
			if (splinePoints_.getVertexCount() == 0)
				return sf::Vector2f();

			c2v pos;
			if (a_time == 1.0f)
				pos = { splinePoints_[splinePoints_.getVertexCount() - 1].position.x, splinePoints_[splinePoints_.getVertexCount() - 1].position.y };
			else if (a_time == 0.0f)
				pos = { splinePoints_[0].position.x, splinePoints_[0].position.y };
			else
			{
				float a_dist = getLength()*a_time;
				indexHint = getSegmentStartIndexAtDist(a_dist, indexHint);
				if (indexHint < 0 || indexHint >= splinePoints_.getVertexCount())
					return sf::Vector2f();

				// segment
				c2v start = { splinePoints_[indexHint].position.x, splinePoints_[indexHint].position.y };
				c2v end = { splinePoints_[indexHint + 1].position.x, splinePoints_[indexHint + 1].position.y };

				// calc rest dist on the segment
				a_dist -= splinePointsLengths_[indexHint];

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
			if (splinePoints_.getVertexCount() < 2)
				return 0.0f;


			c2v start, end, seg;
			if (a_time == 1.0f)
			{
				start = { splinePoints_[splinePoints_.getVertexCount() - 2].position.x, splinePoints_[splinePoints_.getVertexCount() - 2].position.y };
				end = { splinePoints_[splinePoints_.getVertexCount() - 1].position.x, splinePoints_[splinePoints_.getVertexCount() - 1].position.y };
			}
			else if (a_time == 0.0f)
			{
				start = { splinePoints_[0].position.x, splinePoints_[0].position.y };
				end = { splinePoints_[1].position.x, splinePoints_[1].position.y };
			}
			else
			{
				indexHint = getSegmentStartIndexAtDist(getLength()*a_time, indexHint);
				if (indexHint < 0 || indexHint >= splinePoints_.getVertexCount())
					return 0.0f;

				start = { splinePoints_[indexHint].position.x, splinePoints_[indexHint].position.y };
				end = { splinePoints_[indexHint + 1].position.x, splinePoints_[indexHint + 1].position.y };
			}

			seg = c2Sub(end, start);
			// return in radiant or degree
			return a_in_radiant ? atan2(seg.y, seg.x) : atan2(seg.y, seg.x)*RAD_TO_DEG;
		}
		float Spline::getLength()
		{
			if (splinePointsLengths_.size() != splinePoints_.getVertexCount())
				recalcLength();

			if (splinePoints_.getVertexCount() == 0)
				return 0.0f;

			return splinePointsLengths_.back();
		}

		void Spline::recalcLength(unsigned int startindex)
		{
			size_t size = splinePoints_.getVertexCount();
			splinePointsLengths_.resize(size);

			// first point always has length zero
			if (startindex == 0)
			{
				splinePointsLengths_[startindex] = 0;
				startindex++;
			}

			// we can assume here that the startindex is > 0 because of the above increment
			for (int i = startindex; i < size; i++)
				splinePointsLengths_[i] = splinePointsLengths_[i - 1] + c2Len(c2Sub(c2v{ splinePoints_[i].position.x , splinePoints_[i].position.y }, c2v{ splinePoints_[i - 1].position.x , splinePoints_[i - 1].position.y }));
		}


		float Spline::getLengthAtTime(float a_time)
		{
			return getLength() * a_time;
		}
		
		void Spline::appendControlPoint(sf::Vector2f a_pt)
		{
			auto size_before = controlPoints_.getVertexCount();
			controlPoints_.append(sf::Vertex(a_pt, colorControlPts_));

			onControlPointsAdded(size_before);
		}

		bool Spline::getLastControlPoint(sf::Vector2f & a_pt)
		{
			if(controlPoints_.getVertexCount() == 0)
				return false;

			 a_pt = controlPoints_[controlPoints_.getVertexCount() - 1].position;
			 return true;
		}

		bool Spline::getLastControlPointSegment(sf::Vector2f & a_start, sf::Vector2f & a_end)
		{
			if (controlPoints_.getVertexCount() < 2)
				return false;

			a_start = controlPoints_[controlPoints_.getVertexCount() - 2].position;
			a_end	= controlPoints_[controlPoints_.getVertexCount() - 1].position;
			return true;
		}

		sf::Color Spline::getColor()
		{
			return color_;
		}

		void Spline::setColor(sf::Color a_color, bool recolor_existing)
		{
			color_ = a_color;
			if (recolor_existing)
			{
				for (size_t i = 0; i < splinePoints_.getVertexCount(); i++)
					splinePoints_[i].color = color_;
			}	
		}

		sf::Color Spline::getColor_controlPts()
		{
			return colorControlPts_;
		}

		void Spline::setColor_controlPts(sf::Color a_color, bool recolor_existing)
		{
			colorControlPts_ = a_color;
			if (recolor_existing)
			{
				for (size_t i = 0; i < controlPoints_.getVertexCount(); i++)
					splinePoints_[i].color = colorControlPts_;
			}
		}

		void Spline::appendSplinePoint(sf::Vector2f a_pt)
		{
			splinePoints_.append(sf::Vertex(a_pt, color_));

			recalcLength(splinePoints_.getVertexCount() - 1);	
		}

		int Spline::getSegmentStartIndexAtTime(float a_time, int indexHint)
		{
			return getSegmentStartIndexAtDist(a_time / getLength(), indexHint);
		}

		// return -1 in case of an error
		int Spline::getSegmentStartIndexAtDist(float a_dist, int indexHint)
		{
			int i = 0;

			size_t size = splinePoints_.getVertexCount();

			if (size < 0)
				return -1;
			else if (size > 1)
			{
				float len = getLength();

				// take indexHint or make an index guess
				if (indexHint >= 0 && indexHint < size - 1)
					i = indexHint;
				else
					i = c2Max(1, c2Min(((float)(size - 1) * a_dist / len), size - 2));

				// keep index range
				while (i > 0 && i < size - 1)
				{
					if (splinePointsLengths_[i] <= a_dist)
					{
						// found correct segment
						if (splinePointsLengths_[i + 1] > a_dist)
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