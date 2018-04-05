#pragma once
#include <SFML\Graphics.hpp>
#include "MathHelper2D.h"
#include <list>

namespace tgf
{
	namespace utilities
	{
		struct cgSettings
		{
			int road_segAngleRange = 10;					//possible angle in both directions. so -anglerange/2 to + angleRange/2
			float road_segLength = 30.0f;
			float road_chanceToSplitRadius = 1000.0f;		// to generate crossings
			float road_chanceToContinueRadius = 1000.f;		// to stop at maximum 1000.0f (+1*seglen)
			float road_crossingMinDist = 40.0f;
			
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
			std::vector<sf::Vector2f> road_crossings_;
			std::vector<sf::Vector2f> road_deadends_;
		private:
			// general city generation
			void generateRoads();
			void processRoadSegment(roadsegment_candidate & seg);
			void advanceRoadCandidate(roadsegment_candidate & seg, float additional_angle = 0.0f);
			
			// local constraint functions
			bool applyLocalContraintsToSegmentCandidate(roadsegment_candidate & seg);
			bool connectToDeadEndInRadius(roadsegment_candidate & seg, float radius);
			bool connectToExistingRoadSeg_intersecting(roadsegment_candidate & seg);
			bool connectToExistingRoadSeg_extending(roadsegment_candidate & seg, float radius);
			bool connectToExistingCrossing(roadsegment_candidate & seg, float radius);

			// helper functions
			void insertCrossingAtExistingRoadSegment(int i, roadsegment_candidate & seg, sf::Vector2f intersection);
			bool checkForCrossingInRadius(sf::Vector2f & pt, float radius, sf::Vector2f* crossing = NULL);
			int checkForIntersection(roadsegment_candidate & seg, sf::Vector2f & intersecting_pt);
			int extendSegmentOntoExistingRoadSegment(roadsegment_candidate & seg, float maxdist, sf::Vector2f & intersecting_pt);

			cgSettings settings_;
			c2v mid_;

			std::list<roadsegment_candidate> road_candidates_;
		};
	}
}