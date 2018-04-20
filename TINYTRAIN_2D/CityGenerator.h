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
			float constAngle = 0.0;
			float angle = 0.0f;
		};

		struct road_crossing
		{
		public:
			sf::Vector2f pt;
			float angle;

			int roads;
		private:
			
			// crossing slots (right, up, left, down)
			// 0 - right (315, 360]+[0, 45], 1 - up(45, 135], 2 - left(135, 225], 3 - down(225, 315] , can later be shifted by angle
			bool slots[4]{ false };

		public:
			int addRoad(float road_angle)
			{
				int rc = -1;
				int possible_slot_index = crossing_index_from_angle(road_angle);
				if (slots[possible_slot_index] == false)
				{
					rc = possible_slot_index;
					slots[rc] = true;

					updateRoadCount();
				}

				return rc;
			}

			bool removeRoad(float road_angle)
			{
				bool rc = false;
				int possible_slot_index = crossing_index_from_angle(road_angle);
				if (slots[possible_slot_index] == true)
				{
					rc = true;
					slots[possible_slot_index] = false;

					updateRoadCount();
				}

				return rc;
			}

			int crossing_index_from_angle(float road_angle)
			{
				// bring given angle into [0, 360) range (+45 + crossing.angle)
				road_angle = fmod(road_angle + 45.0f + angle, 360.0f);
				if (road_angle < 0.0f)
					road_angle += 360.0f;

				int slot_index = road_angle / 90.0f;
				return slot_index;
			}

			int updateRoadCount()
			{
				roads = 0;
				for (bool& r : slots)
					if (r) roads++;
				
				return roads;
			}

			road_crossing()
			{
				pt = sf::Vector2f();
				roads = 0;
				angle = 0.0f;
			}

			road_crossing(sf::Vector2f a_pt, float a_angle = 0.0f)
			{
				pt = a_pt;
				angle = a_angle;
				roads = 0;
			}
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
			std::vector<road_crossing> road_crossings_;
			std::vector<sf::Vector2f> road_deadends_;
		private:
			// general city generation
			void generateRoads();
			void processRoadSegment(roadsegment_candidate & seg);
			roadsegment_candidate advanceRoadCandidate(roadsegment_candidate & seg, float additional_angle = 0.0f);
			
			// local constraint functions
			bool applyLocalContraintsToSegmentCandidate(roadsegment_candidate & seg);
			bool connectToDeadEndInRadius(roadsegment_candidate & seg, float radius);
			bool connectToCandidateStartInRadius(roadsegment_candidate & seg, float radius, float angle_tolerance);
			bool connectToExistingRoadSeg_intersecting(roadsegment_candidate & seg);
			bool connectToExistingRoadSeg_extending(roadsegment_candidate & seg, float radius);
			bool connectToExistingCrossing(roadsegment_candidate & seg, float radius);

			// helper functions
			bool insertCrossingAtExistingRoadSegment(int i, roadsegment_candidate & seg, sf::Vector2f intersection);
			bool checkForCrossingInRadius(sf::Vector2f & pt, float radius, road_crossing* crossing = NULL);
			int checkForIntersection(roadsegment_candidate & seg, sf::Vector2f & intersecting_pt);
			int extendSegmentOntoExistingRoadSegment(roadsegment_candidate & seg, float maxdist, sf::Vector2f & intersecting_pt);

			cgSettings settings_;
			c2v mid_;

			std::list<roadsegment_candidate> road_candidates_;
		};
	}
}