#define TINYC2_IMPLEMENTATION
#include "tinyc2.h"

#include "TRailTrack.h"
#include "TTrain.h"

namespace tinytrain
{
	TRailTrack::TRailTrack()
	{
		segLength_ = 100.0f;

		trackspline_ = std::make_unique<tgf::math::Spline_CatmullRom>();
		trackspline_->type_ = tgf::math::Spline_CatmullRom::CatmullRomType::Chordal;
		trackspline_->drawControlPoints_ = false;
	}

	TRailTrack::~TRailTrack()
	{
	}
	
	void TRailTrack::append(const sf::Vector2f& a_ctrlPt)
	{
		if (trackspline_)
			trackspline_->appendControlPoint(a_ctrlPt);
	}

	void TRailTrack::addTrain(TTrain * a_train, float a_atDistance)
	{
		trains_.push_back(a_train);
		if (a_atDistance >= 0.0f && a_atDistance < trackspline_->getLength())
			a_train->distance_ = a_atDistance;
		else
			a_train->distance_ = 0.0f;
	}

	void TRailTrack::moveAndRotateOnRail(TTrain* train)
	{
		// max dist to travel on the railtrack
		float maxdist = trackspline_->getLength();

		if (train->distance_ > maxdist)
		{
			// todo: event of the train reaching the end of its railtrack (ONLY FIRST WAGON IS CONCERNED)
			train->distance_ = maxdist;
		}
		else if (train->distance_ < 0.0f)
		{
			// todo: event of invalid distance (ONLY FIRST WAGON IS CONCERNED)
			train->distance_ = 0.0f;
		}


		int hintindex = -1;
		for (int i = 0; i < train->wagons_.size(); i++)
		{
			// calc position of the current wagon
			float current_wagon_dist = train->distance_ - i * (train->wagonsize_.x + train->wagongap_);
			float time = current_wagon_dist / trackspline_->getLength();

			//todo add hintindex
			float angle = trackspline_->getDirectionAngleAtTime(time, hintindex, false);
			sf::Vector2f pos = trackspline_->getLocationAtTime(time, hintindex);

			//hintindex = getSegmentStartIndexAtDist(...)
			//setPositionAndRotationFromRail(current_wagon_dist, hintindex, &train->wagons_[i]);


			train->wagons_[i].setPosition(pos);
			train->wagons_[i].setRotation(angle);
		}
	}
	/*
	void TRailTrack::setPositionAndRotationFromRail(float a_dist, int i, sf::Transformable* obj)
	{
		sf::Vector2f pos;
		float angle = 0.0f;
		size_t size = trackspline_.getVertexCount();
		if (size)
		{
			// outside of railrange
			if (i + 1 >= size)
				i--;

			if (i < 0)
				i = 0;

			if (i + 1 < size)
			{
				c2v start{ trackspline_[i].position.x,		trackspline_[i].position.y };
				c2v end{ trackspline_[i + 1].position.x,	trackspline_[i + 1].position.y };

				c2v seg = c2Sub(end, start);
				// 57.295779513 := rad to degre conversion (rad * 180.0/pi)
				angle = atan2(seg.y, seg.x) * RAD_TO_DEG;


				if (length_[i] == a_dist)
				{
					pos = trackspline_[i].position;
				}
				else
				{
					float seg_len = length_[i + 1] - length_[i];
					float alpha_on_seg = (a_dist - length_[i]) / seg_len;

					c2v temp = c2Lerp(start, end, alpha_on_seg);
					pos.x = temp.x;
					pos.y = temp.y;
				}
			}
			// something strange happend.. probably just vertex on the rail
			else
			{
				pos = trackspline_[i].position;
			}

			obj->setPosition(pos);
			obj->setRotation(angle);
		}
	}*/

	float TRailTrack::getSegmentLength()
	{
		return segLength_;
	}

	void TRailTrack::setSegmentLength(float a_len)
	{
		segLength_ = a_len;
	}

	bool TRailTrack::getLastControlPointFromTrack(sf::Vector2f & a_pt)
	{
		if (trackspline_)
			return trackspline_->getLastControlPoint(a_pt);

		return false;
	}

	bool TRailTrack::getLastControlPointSegmentFromTrack(sf::Vector2f & a_start, sf::Vector2f & a_end)
	{
		if (trackspline_)
			return trackspline_->getLastControlPointSegment(a_start, a_end);

		return false;
	}

	void TRailTrack::addDrawnLinePoints(std::vector<sf::Vector2f> a_points, sf::Color a_color)
	{
		int i = 0;
		if (a_points.size())
		{
			// check wether to skip the first new point because it is the same as the previous last point on the spline
			sf::Vector2f end;
			if(trackspline_->getLastControlPoint(end) && end == a_points[0])
				i++;
		}

		for (; i < a_points.size(); i++)
			trackspline_->appendControlPoint(a_points[i]);
	}

	void TRailTrack::draw(sf::RenderTarget * target)
	{
		trackspline_->draw(target);
	}

	void TRailTrack::update(float deltaTime)
	{
		// move all the trains on the track
		for (int i = 0; i < trains_.size(); i++)
			moveAndRotateOnRail(trains_[i]);
	}
}