#define TINYC2_IMPLEMENTATION
#include "tinyc2.h"

#include "TRailTrack.h"
#include "TTrain.h"

namespace tinytrain
{
	TRailTrack::TRailTrack()
	{
		// default railtrack draw type
		m_trackspline.setPrimitiveType(sf::PrimitiveType::LinesStrip);

		m_segLength = 100.0f;
	}

	TRailTrack::~TRailTrack()
	{
	}

	void TRailTrack::recalcLength(unsigned int startindex)
	{
		size_t size = m_trackspline.getVertexCount();
		m_length.resize(size);

		// first point always has length zero
		if (startindex == 0)
		{
			m_length[startindex] = 0;
			startindex++;
		}

		// we can assume here that the startindex is > 0 because of the above
		for (int i = startindex; i < size; i++)
			m_length[i] = m_length[i - 1] + c2Len(c2Sub(c2v{ m_trackspline[i].position.x , m_trackspline[i].position.y }, c2v{ m_trackspline[i - 1].position.x , m_trackspline[i - 1].position.y }));
	}

	void TRailTrack::append(const sf::Vertex & vertex)
	{
		m_trackspline.append(vertex);

		// calc length for the last vertex
		recalcLength(m_trackspline.getVertexCount() - 1);
	}

	float TRailTrack::getLength()
	{
		if (m_length.size() != m_trackspline.getVertexCount())
			recalcLength();

		return m_length[m_trackspline.getVertexCount() - 1];
	}

	void TRailTrack::addTrain(TTrain * a_train, float a_atDistance)
	{
		m_trains.push_back(a_train);
		if (a_atDistance >= 0.0f && a_atDistance < getLength())
			a_train->m_distance = a_atDistance;
		else
			a_train->m_distance = 0.0f;
	}

	void TRailTrack::moveAndRotateOnRail(TTrain* train)
	{
		// max dist to travel on the railtrack
		float maxdist = getLength();

		if (train->m_distance > maxdist)
		{
			// todo: event of the train reaching the end of its railtrack (ONLY FIRST WAGON IS CONCERNED)
			train->m_distance = maxdist;
		}
		else if (train->m_distance < 0.0f)
		{
			// todo: event of invalid distance (ONLY FIRST WAGON IS CONCERNED)
			train->m_distance = 0.0f;
		}


		int hintindex = -1;
		for (int i = 0; i < train->m_wagons.size(); i++)
		{
			// calc position of the current wagon
			float current_wagon_dist = train->m_distance - i * (train->m_wagonsize.x + train->m_wagongap);
			hintindex = getSegmentStartIndexAtDist(current_wagon_dist, hintindex);

			setPositionAndRotationFromRail(current_wagon_dist, hintindex, &train->m_wagons[i]);
		}
	}

	void TRailTrack::setPositionAndRotationFromRail(float a_dist, int i, sf::Transformable* obj)
	{
		sf::Vector2f pos;
		float angle = 0.0f;
		size_t size = m_trackspline.getVertexCount();
		if (size)
		{
			// outside of railrange
			if (i + 1 >= size)
				i--;

			if (i < 0)
				i = 0;

			if (i + 1 < size)
			{
				c2v start{ m_trackspline[i].position.x,		m_trackspline[i].position.y };
				c2v end{ m_trackspline[i + 1].position.x,	m_trackspline[i + 1].position.y };

				c2v seg = c2Sub(end, start);
				// 57.295779513 := rad to degre conversion (rad * 180.0/pi)
				angle = atan2(seg.y, seg.x) * RAD_TO_DEG;


				if (m_length[i] == a_dist)
				{
					pos = m_trackspline[i].position;
				}
				else
				{
					float seg_len = m_length[i + 1] - m_length[i];
					float alpha_on_seg = (a_dist - m_length[i]) / seg_len;

					c2v temp = c2Lerp(start, end, alpha_on_seg);
					pos.x = temp.x;
					pos.y = temp.y;
				}
			}
			// something strange happend.. probably just vertex on the rail
			else
			{
				pos = m_trackspline[i].position;
			}

			obj->setPosition(pos);
			obj->setRotation(angle);
		}

	}

	// return -1 in case of an error
	int TRailTrack::getSegmentStartIndexAtDist(float a_dist, int indexHint)
	{
		int i = 0;

		size_t size = m_trackspline.getVertexCount();

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
				if (m_length[i] <= a_dist)
				{
					// found correct segment
					if (m_length[i + 1] > a_dist)
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

	float TRailTrack::getDirectionAngleAtTime(float a_time, bool rad)
	{
		// no direction calc possible when not enough points
		if (m_trackspline.getVertexCount() < 2) 
			return 0.0f;


		c2v start, end, seg;
		if (a_time == 1.0f)
		{
			start = { m_trackspline[m_trackspline.getVertexCount() - 2].position.x, m_trackspline[m_trackspline.getVertexCount() - 2].position.y };
			end =	{ m_trackspline[m_trackspline.getVertexCount() - 1].position.x, m_trackspline[m_trackspline.getVertexCount() - 1].position.y };
		}
		else if (a_time == 0.0f)
		{
			start = { m_trackspline[0].position.x, m_trackspline[0].position.y };
			end =	{ m_trackspline[1].position.x, m_trackspline[1].position.y };
		}
		else
		{
			int segindex = getSegmentStartIndexAtDist(getLength()*a_time);
			if (segindex < 0 || segindex >= m_trackspline.getVertexCount())
				return 0.0f;
			
			start = { m_trackspline[segindex].position.x, m_trackspline[segindex].position.y };
			end =	{ m_trackspline[segindex + 1].position.x, m_trackspline[segindex + 1].position.y };
		}

		seg		= c2Sub(end, start);
		return rad? atan2(seg.y, seg.x) : atan2(seg.y, seg.x)*RAD_TO_DEG;
	}

	sf::Vector2f TRailTrack::getLocationAtTime(float a_time)
	{
		if (m_trackspline.getVertexCount() == 0)
			return sf::Vector2f();

		c2v pos;
		if (a_time == 1.0f)
			pos = { m_trackspline[m_trackspline.getVertexCount() - 1].position.x, m_trackspline[m_trackspline.getVertexCount() - 1].position.y };
		else if (a_time == 0.0f)
			pos = { m_trackspline[0].position.x, m_trackspline[0].position.y };
		else
		{
			float a_dist = getLength()*a_time;
			int segindex = getSegmentStartIndexAtDist(a_dist);
			if (segindex < 0 || segindex >= m_trackspline.getVertexCount())
				return sf::Vector2f();

			// segment
			c2v start = { m_trackspline[segindex].position.x, m_trackspline[segindex].position.y };
			c2v end =	{ m_trackspline[segindex + 1].position.x, m_trackspline[segindex + 1].position.y };

			// calc rest dist on the segment
			a_dist -= m_length[segindex];

			// make it 0-1 range based on the segment
			a_dist = a_dist / c2Len(c2Sub(end, start));
			pos = c2Lerp(start, end, a_dist);
		}
		
		return sf::Vector2f(pos.x, pos.y);
	}

	float TRailTrack::getSegmentLength()
	{
		return m_segLength;
	}

	void TRailTrack::setSegmentLength(float a_len)
	{
		m_segLength = a_len;
	}

	void TRailTrack::addDrawnLinePoints(std::vector<sf::Vector2f> a_points, sf::Color a_color)
	{
		int i = 0;
		if (m_trackspline.getVertexCount() && a_points.size())
		{
			// check wether to skip the first new point because it is the same as the previous last point on the spline
			if (m_trackspline[m_trackspline.getVertexCount() - 1].position == a_points[0])
				i++;
		}

		for (; i < a_points.size(); i++)
			append(sf::Vertex(a_points[i], a_color));
	}

	void TRailTrack::draw(sf::RenderTarget * target)
	{
		target->draw(m_trackspline);
	}

	void TRailTrack::update(float deltaTime)
	{
		// move all the trains on the track
		for (int i = 0; i < m_trains.size(); i++)
			moveAndRotateOnRail(m_trains[i]);
	}
}