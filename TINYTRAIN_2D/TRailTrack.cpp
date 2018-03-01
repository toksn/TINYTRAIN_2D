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

			float angle = trackspline_->getDirectionAngleAtTime(time, hintindex, false);
			sf::Vector2f pos = trackspline_->getLocationAtTime(time, hintindex);

			//hintindex = getSegmentStartIndexAtDist(...)
			//setPositionAndRotationFromRail(current_wagon_dist, hintindex, &train->wagons_[i]);


			train->wagons_[i].setPosition(pos);
			train->wagons_[i].setRotation(angle);
		}
	}

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

	void TRailTrack::addDrawnLinePoints(std::vector<sf::Vector2f> a_points)
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

		onSplineChanged();
	}

	tgf::math::Spline_CatmullRom * TRailTrack::getTrackSpline()
	{
		return trackspline_.get();
	}

	void TRailTrack::onSplineChanged()
	{
		for (auto f : trackChangedCallbacks_)
			f.first();
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