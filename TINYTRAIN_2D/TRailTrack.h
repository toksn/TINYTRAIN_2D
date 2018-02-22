#pragma once
#include "Entity.h"
#include "Spline_CatmullRom.h"
#include <memory>
#include <functional>
#include <SFML\Graphics.hpp>

namespace tinytrain
{
	class TTrain;

	class TRailTrack : public tgf::Entity
	{
	public:
		TRailTrack();
		~TRailTrack();

		void append(const sf::Vector2f & a_ctrlPt);

		// Inherited via Entity
		virtual void draw(sf::RenderTarget * target) override;
		virtual void update(float deltaTime) override;

		// functions for moving trains along the track
		void addTrain(TTrain* a_train, float a_atDistance = 0.0f);
		void moveAndRotateOnRail(TTrain * train);
		//int getSegmentStartIndexAtDist(float a_dist, int indexHint = -1);


		float getSegmentLength();
		void setSegmentLength(float a_len);
		bool getLastControlPointFromTrack(sf::Vector2f& a_pt);
		bool getLastControlPointSegmentFromTrack(sf::Vector2f& a_start, sf::Vector2f& a_end);
		void addDrawnLinePoints(std::vector<sf::Vector2f> a_points, sf::Color a_color);

		tgf::math::Spline_CatmullRom* getTrackSpline();

		template<class T> void bindTrackChangedCallback(T* const object, void(T::* const mf)(void))
		{
			// save a pair of the acutal function call and the object pointer for callback deletion
			trackChangedCallbacks_.push_back(std::make_pair<std::function<void(void)>, void*>(std::bind(mf, object), &*object));
		}
		template<class T> void unbindAllCallbacks(T* const object)
		{
			for (auto f = trackChangedCallbacks_.begin(); f != trackChangedCallbacks_.end(); ++f)
			{
				if (f->second == object)
				{
					trackChangedCallbacks_.erase(f);
					return;
				}
			}
		}

		// contains the length at each of the vertices
		std::vector<float> length_;

		// array of trains actually driving on the track
		std::vector<TTrain*> trains_;
				
	private:
		void onSplineChanged();

		std::vector<std::pair<std::function<void(void)>, void*>> trackChangedCallbacks_;
		float segLength_;
		std::unique_ptr<tgf::math::Spline_CatmullRom> trackspline_;
	};
}