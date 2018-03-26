#pragma once

#include <SFML\Graphics.hpp>
#include <vector>

namespace tgf
{
	namespace utilities
	{
		struct cgSettings
		{
			float road_segAngleRange = 5.0f;  //possible angle in both directions. so -anglerange/2 to + angleRange/2
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
			//CityGenerator();
			//~CityGenerator();

			void applySettings(cgSettings settings);
			cgSettings getSettings();

			void generate();

		private:
			void generateRoads();

			void processRoadSegment(roadsegment_candidate & seg);


			cgSettings settings_;

			std::vector<roadsegment_candidate> road_candidates_;
			sf::VertexArray road_segments_;

			std::vector<sf::Vector2f> road_crossings_;
		};
	}
}