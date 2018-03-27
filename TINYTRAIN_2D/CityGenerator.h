#pragma once
#include <SFML\Graphics.hpp>
#include "tinyc2.h"
#include <list>

namespace tgf
{
	namespace utilities
	{
		struct cgSettings
		{
			int road_segAngleRange = 10;						//possible angle in both directions. so -anglerange/2 to + angleRange/2
			float road_segLength = 50.0f;
			float road_chanceToSplitRadius = 1000.0f;		// to generate crossings
			float road_chanceToContinueRadius = 1000.f;		// to stop at maximum 1000.0f (+1*seglen)
			
			sf::Vector2f road_startingPoint;
		};

		struct roadsegment_candidate
		{
			sf::Vector2f a;
			sf::Vector2f b;

			// chance for const angle
			bool constAngle = false;
			float angle = 0.0f;
		};

		class CityGenerator
		{
		public:
			CityGenerator();
			~CityGenerator();

			void applySettings(cgSettings settings);
			cgSettings getSettings();

			void generate();

			sf::VertexArray road_segments_;
		private:
			void generateRoads();
			void processRoadSegment(roadsegment_candidate & seg);

			void advanceRoadCandidate(roadsegment_candidate & seg, float additional_angle = 0.0f);

			cgSettings settings_;
			c2v mid_;

			std::list<roadsegment_candidate> road_candidates_;
			std::vector<sf::Vector2f> road_crossings_;
		};
	}
}