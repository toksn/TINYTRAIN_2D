#define TINYC2_IMPLEMENTATION
#include "tinyc2.h"

#include "TRailTrack.h"
#include "TTrain.h"
#include "Spline_CatmullRom.h"

namespace tinytrain
{
	TRailTrack::TRailTrack()
	{
		m_segLength = 100.0f;

		m_trackspline = std::make_unique<tgf::math::Spline_CatmullRom>();
	}

	TRailTrack::~TRailTrack()
	{
	}
	
	void TRailTrack::append(const sf::Vector2f& a_ctrlPt)
	{
		if (m_trackspline)
			m_trackspline->appendControlPoint(a_ctrlPt);
	}

	void TRailTrack::addTrain(TTrain * a_train, float a_atDistance)
	{
		m_trains.push_back(a_train);
		if (a_atDistance >= 0.0f && a_atDistance < m_trackspline->getLength())
			a_train->m_distance = a_atDistance;
		else
			a_train->m_distance = 0.0f;
	}

	void TRailTrack::moveAndRotateOnRail(TTrain* train)
	{
		// max dist to travel on the railtrack
		float maxdist = m_trackspline->getLength();

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
			float time = current_wagon_dist / m_trackspline->getLength();

			//todo add hintindex
			float angle = m_trackspline->getDirectionAngleAtTime(time, hintindex, false);
			sf::Vector2f pos = m_trackspline->getLocationAtTime(time, hintindex);

			//hintindex = getSegmentStartIndexAtDist(...)
			//setPositionAndRotationFromRail(current_wagon_dist, hintindex, &train->m_wagons[i]);


			train->m_wagons[i].setPosition(pos);
			train->m_wagons[i].setRotation(angle);
		}
	}
	/*
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
	}*/

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
		if (a_points.size())
		{
			// check wether to skip the first new point because it is the same as the previous last point on the spline
			sf::Vector2f end;
			if(m_trackspline->getLastControlPoint(end) && end == a_points[0])
				i++;
		}

		for (; i < a_points.size(); i++)
			m_trackspline->appendControlPoint(a_points[i]);
	}

	void TRailTrack::draw(sf::RenderTarget * target)
	{
		m_trackspline->draw(target);
	}

	void TRailTrack::update(float deltaTime)
	{
		// move all the trains on the track
		for (int i = 0; i < m_trains.size(); i++)
			moveAndRotateOnRail(m_trains[i]);
	}
}