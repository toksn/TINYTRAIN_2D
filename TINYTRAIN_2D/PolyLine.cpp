#include "PolyLine.h"
#include "tgfdefines.h"
#include <tinyc2.h>

namespace tgf
{
	namespace math
	{
		PolyLine::PolyLine()
		{
		}
		PolyLine::~PolyLine()
		{
		}

		sf::Vector2f PolyLine::getLocationAtTime(float a_time)
		{
			int index = -1;
			return getLocationAtTime(a_time, index);
		}

		sf::Vector2f PolyLine::getLocationAtTime(float a_time, int& indexHint)
		{
			if (poly_.size() == 0)
				return sf::Vector2f();

			c2v pos;
			if (a_time == 1.0f)
				pos = { poly_[poly_.size() - 1].x, poly_[poly_.size() - 1].y };
			else if (a_time == 0.0f)
				pos = { poly_[0].x, poly_[0].y };
			else
			{
				float a_dist = getLength()*a_time;
				indexHint = getSegmentStartIndexAtDist(a_dist, indexHint);
				if (indexHint < 0 || indexHint >= poly_.size()-1)
					return sf::Vector2f();

				// segment
				c2v start = { poly_[indexHint].x, poly_[indexHint].y };
				c2v end = { poly_[indexHint + 1].x, poly_[indexHint + 1].y };

				// calc rest dist on the segment
				a_dist -= lengths_[indexHint];

				// make it 0-1 range based on the segment
				a_dist = a_dist / c2Len(c2Sub(end, start));
				pos = c2Lerp(start, end, a_dist);
			}

			return sf::Vector2f(pos.x, pos.y);
		}

		float PolyLine::getDirectionAngleAtTime(float a_time, bool a_in_radiant)
		{
			int index = -1;
			return getDirectionAngleAtTime(a_time, index, a_in_radiant);
		}

		float PolyLine::getDirectionAngleAtTime(float a_time, int& indexHint, bool a_in_radiant)
		{
			// no direction calc possible when not enough points
			if (poly_.size() < 2)
				return 0.0f;


			c2v start, end, seg;
			if (a_time == 1.0f)
			{
				start = { poly_[poly_.size() - 2].x, poly_[poly_.size() - 2].y };
				end = { poly_[poly_.size() - 1].x, poly_[poly_.size() - 1].y };
			}
			else if (a_time == 0.0f)
			{
				start = { poly_[0].x, poly_[0].y };
				end = { poly_[1].x, poly_[1].y };
			}
			else
			{
				indexHint = getSegmentStartIndexAtDist(getLength()*a_time, indexHint);
				if (indexHint < 0 || indexHint >= poly_.size()-1)
					return 0.0f;

				start = { poly_[indexHint].x, poly_[indexHint].y };
				end = { poly_[indexHint + 1].x, poly_[indexHint + 1].y };
			}

			seg = c2Sub(end, start);
			// return in radiant or degree
			return a_in_radiant ? atan2(seg.y, seg.x) : atan2(seg.y, seg.x)*RAD_TO_DEG;
		}

		float PolyLine::getLength()
		{
			if (lengths_.size() != poly_.size())
				recalcLength();

			if (poly_.size() == 0)
				return 0.0f;

			return lengths_.back();
		}

		void PolyLine::recalcLength(unsigned int startindex)
		{
			size_t size = poly_.size();
			lengths_.resize(size);

			if (size == 0)
				return;

			// first point always has length zero
			if (startindex == 0)
			{
				lengths_[startindex] = 0;
				startindex++;
			}

			// we can assume here that the startindex is > 0 because of the above increment
			for (int i = startindex; i < size; i++)
				lengths_[i] = lengths_[i - 1] + c2Len(c2Sub(c2v{ poly_[i].x , poly_[i].y }, c2v{ poly_[i - 1].x , poly_[i - 1].y }));
		}

		float PolyLine::getLengthAtTime(float a_time)
		{
			return getLength() * a_time;
		}

		int PolyLine::getSegmentStartIndexAtTime(float a_time, int indexHint)
		{
			return getSegmentStartIndexAtDist(a_time / getLength(), indexHint);
		}
		int PolyLine::getSegmentStartIndexAtDist(float a_dist, int indexHint)
		{
			int i = 0;

			size_t size = poly_.size();

			if (size < 2)
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
					if (lengths_[i] <= a_dist)
					{
						// found correct segment
						if (lengths_[i + 1] > a_dist)
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