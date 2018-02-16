#include "TT_RailRoad.h"
#include "TT_Train.h"

#define TINYC2_IMPLEMENTATION
#include "tinyc2.h"

TT_RailRoad::TT_RailRoad(EntityManager* man) : Entity(man)
{
	// default railroad draw type
	m_trackspline.setPrimitiveType(sf::PrimitiveType::LinesStrip);
}

TT_RailRoad::~TT_RailRoad()
{
}

void TT_RailRoad::recalcLength(unsigned int startindex)
{
	int size = m_trackspline.getVertexCount();
	m_length.resize(size);
	
	// first point always has length zero
	if (startindex == 0)
	{
		m_length[startindex] = 0;
		startindex++;
	}

	// we can assume here that the startindex is > 0 because of the above
	for (int i = startindex; i < size; i++)
		m_length[i] = m_length[i-1] + c2Len(c2Sub(c2v{ m_trackspline[i].position.x , m_trackspline[i].position.y }, c2v{ m_trackspline[i-1].position.x , m_trackspline[i-1].position.y }));
}

void TT_RailRoad::append(const sf::Vertex & vertex)
{
	m_trackspline.append(vertex);

	// calc length for the last vertex
	recalcLength(m_trackspline.getVertexCount() - 1);
}

float TT_RailRoad::getLength()
{
	if (m_length.size() != m_trackspline.getVertexCount())
		recalcLength();

	return m_length[m_trackspline.getVertexCount()-1];
}

void TT_RailRoad::addTrain(TT_Train * a_train, float a_atDistance)
{
	m_trains.push_back(a_train);
	if (a_atDistance >= 0.0f && a_atDistance < getLength())
		a_train->m_distance = a_atDistance;
	else
		a_train->m_distance = 0.0f;
}

void TT_RailRoad::moveAndRotateOnRail(TT_Train* train)
{
	float dist = train->m_distance;
	// max dist to travel on the railroad
	float maxdist = getLength();

	if (dist > maxdist)
	{
		// todo: event of the train reaching the end of its railroad

		dist = maxdist;
	}
	else if (dist < 0.0f)
	{
		// todo: event of invalid distance
		dist = 0.0f;
	}


	int hintindex = -1;
	for (int i = 0; i < train->m_wagons.size(); i++)
	{
		// calc position of the current wagon
		float current_wagon_dist = dist - i * (train->m_wagonsize.x + train->m_wagongap);
		hintindex = getSegmentStartIndexAtDist(current_wagon_dist, hintindex);

		setPositionAndRotationFromRail(current_wagon_dist, hintindex, &train->m_wagons[i]);
	}
}

void TT_RailRoad::setPositionAndRotationFromRail(float a_dist, int i, sf::Transformable* obj)
{
	sf::Vector2f pos;
	float angle = 0.0f;
	int size = m_trackspline.getVertexCount();
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
			angle = atan2(seg.y, seg.x) * 57.295779513f;
		

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
/*
sf::Vector2f TT_RailRoad::getPositionOnRail(float a_dist, int index)
{
	sf::Vector2f pos;

	//todo: extrapolate first segment to negative distance
	if (a_dist < 0.0f)
		a_dist = 0.0f;

	int size = m_trackspline.getVertexCount();
	if (size)
	{
		float len = getLength();

		//todo: extrapolate last segment to over actual railroad length
		if (a_dist > len)
			a_dist = len;

		// find segment index
		int i = getSegmentStartIndexAtDist(a_dist, index);

		if(i+1<size)
		{
			if (m_length[i] == a_dist)
			{
				pos = m_trackspline[i].position;
			}
			else
			{
				float seg_len = m_length[i + 1] - m_length[i];
				float alpha_on_seg = (a_dist - m_length[i]) / seg_len;

				c2v start{ m_trackspline[i].position.x,		m_trackspline[i].position.y };
				c2v end{ m_trackspline[i+1].position.x,		m_trackspline[i+1].position.y };
				c2v temp = c2Lerp(start, end, alpha_on_seg);
				pos.x = temp.x;
				pos.y = temp.y;
			}
		}
		// something strange happend.. probably just vertex on the rail
		else
			pos = m_trackspline[i].position;
	}

	return pos;
}
*/
int TT_RailRoad::getSegmentStartIndexAtDist(float a_dist, int indexHint)
{
	int i = 0;

	int size = m_trackspline.getVertexCount();

	if (size < 0)
		return -1;
	else if (size > 1)
	{
		float len = getLength();

		// take indexHint or make an index guess
		if (indexHint >= 0 && indexHint < size - 1)
			i = indexHint;
		else
			i = c2Min(((float)(size-1) * a_dist / len), size-2);

		// keep index range
		while (i >= 0 && i < size-1)
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

void TT_RailRoad::draw(sf::RenderWindow & target)
{
	target.draw(m_trackspline);
}

void TT_RailRoad::update(float deltaTime)
{
	// move all the trains on the track
	for (int i = 0; i < m_trains.size(); i++)
		moveAndRotateOnRail(m_trains[i]);
}