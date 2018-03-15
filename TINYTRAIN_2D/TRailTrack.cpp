#define TINYC2_IMPLEMENTATION
#include "tinyc2.h"

#include "TRailTrack.h"
#include "TTrain.h"

namespace tinytrain
{
	TRailTrack::TRailTrack()
	{
		segLength_ = 100.0f;

		track_ = std::make_unique<tgf::utilities::SplineTexture>();
		track_->width_ = 20.0f;

		track_->spline_->type_ = tgf::math::Spline_CatmullRom::CatmullRomType::Chordal;
		track_->spline_->drawControlPoints_ = false;
		track_->spline_->interpolateControlPointEnds_ = true;
	}

	TRailTrack::~TRailTrack()
	{
	}
	
	void TRailTrack::append(const sf::Vector2f& a_ctrlPt)
	{
		if (track_->spline_)
			track_->spline_->appendControlPoint(a_ctrlPt);
	}

	void TRailTrack::addTrain(TTrain * a_train, float a_atDistance)
	{
		trains_.push_back(a_train);
		if (a_atDistance >= 0.0f && a_atDistance < track_->spline_->getLength())
			a_train->distance_ = a_atDistance;
		else
			a_train->distance_ = 0.0f;
	}

	void TRailTrack::moveAndRotateOnRail(TTrain* train)
	{
		// max dist to travel on the railtrack
		float maxdist = track_->spline_->getLength();

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
			float time = current_wagon_dist / track_->spline_->getLength();

			float angle = track_->spline_->getDirectionAngleAtTime(time, hintindex, false);
			sf::Vector2f pos = track_->spline_->getLocationAtTime(time, hintindex);

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
		if (track_->spline_)
			return track_->spline_->getLastControlPoint(a_pt);

		return false;
	}

	bool TRailTrack::getLastControlPointSegmentFromTrack(sf::Vector2f & a_start, sf::Vector2f & a_end)
	{
		if (track_->spline_)
			return track_->spline_->getLastControlPointSegment(a_start, a_end);

		return false;
	}

	void TRailTrack::addDrawnLinePoints(std::vector<sf::Vector2f> a_points)
	{
		int i = 0;
		if (a_points.size())
		{
			// check wether to skip the first new point because it is the same as the previous last point on the spline
			sf::Vector2f end;
			if(track_->spline_->getLastControlPoint(end) && end == a_points[0])
				i++;
		}

		for (; i < a_points.size(); i++)
			track_->spline_->appendControlPoint(a_points[i]);

		onSplineChanged();
	}

	tgf::math::Spline_CatmullRom * TRailTrack::getTrackSpline()
	{
		return track_->spline_.get();
	}

	void TRailTrack::onSplineChanged()
	{
		for (auto f : trackChangedCallbacks_)
			f.first();
	}

	void TRailTrack::onDraw(sf::RenderTarget * target)
	{
		track_->draw(target);
		
		// this can be used to draw the last control point segment
		//sf::Vertex line[2];
		//if (track_->spline_->getLastControlPointSegment(line[0].position, line[1].position))
		//	target->draw(line, 2, sf::PrimitiveType::LineStrip);
	}

	void TRailTrack::onUpdate(float deltaTime)
	{
		track_->update(deltaTime);

		// move all the trains on the track
		for (int i = 0; i < trains_.size(); i++)
			moveAndRotateOnRail(trains_[i]);
	}
}