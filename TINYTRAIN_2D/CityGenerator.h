#pragma once
#include <SFML\Graphics.hpp>
#include "MathHelper2D.h"
#include <list>
#include <memory>

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

		struct roadsegment
		{
			sf::Vector2f a;
			sf::Vector2f b;

			sf::Color col_a = sf::Color::Green;
			sf::Color col_b = sf::Color::Green;

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
			std::weak_ptr<roadsegment> slots[4];

		public:
			int addRoad(std::shared_ptr<roadsegment>& road)
			{
				// first road to add -> determine crossing angle
				if (roads == 0)
				{
					//calc crossing angle
					float a = road->angle;
					if (road->b == pt)
						a += 180.0f;

					angle = fmod(a+ 45.0f, 90.0f);
					if (angle < 0.0f)
						angle += 90.0f;
					angle -= 45.0f;
					// rotate by another 90.0f degree to match the texture (because it is going top-bottom)
					angle += 90.0f;
				}

				int rc = -1;
				int possible_slot_index = crossing_index_from_angle(road.get());
				if (slots[possible_slot_index].expired())
				{
					rc = possible_slot_index;
					slots[rc] = road;

					updateRoadCount();
				}

				return rc;
			}

			bool removeRoad(std::shared_ptr<roadsegment>& road)
			{
				bool rc = false;
				int possible_slot_index = crossing_index_from_angle(road.get());
				if (slots[possible_slot_index].expired() == false)
				{
					rc = true;
					slots[possible_slot_index].reset();

					updateRoadCount();
				}

				return rc;
			}

			int crossing_index_from_angle(roadsegment* road)
			{
				float road_angle = road->angle;
				//if (road.a.x != pt.x && road.a.y != pt.y && road.b.x == pt)
				if (road->a != pt)
				{
					if (road->b == pt)
						road_angle -= 180.0f;
					else
						printf("crossing_failure: roadsegment is not connected to crossing\n");		// should not happen
				}				

				// treat seg angle in 0-360 range
				road_angle = fmod(road_angle, 360.0f);
				if (road_angle < 0.0f)
					road_angle += 360.0f;

				// rotate by 45 and use crossing angle
				road_angle += 45.0f - angle;
				
				// bring angle into 0-360 range again
				road_angle = fmod(road_angle, 360.0f);
				if (road_angle < 0.0f)
					road_angle += 360.0f;

				int slot_index = road_angle / 90.0f;
				return slot_index;
			}

			int updateRoadCount()
			{
				roads = 0;
				for (auto& r : slots)
					if (r.expired() == false) roads++;
				
				return roads;
			}

			road_crossing()
			{
				pt = sf::Vector2f();
				roads = 0;
				angle = 0.0f;
			}

			road_crossing(sf::Vector2f a_pt)
			{
				pt = a_pt;
				angle = 0.0f;
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

			//sf::VertexArray road_segments_;
			std::list<std::shared_ptr<roadsegment>> road_segments_;
			std::vector<road_crossing> road_crossings_;
			std::vector<sf::Vector2f> road_deadends_;
		private:
			// general city generation
			void generateRoads();
			void processRoadSegment(std::shared_ptr<roadsegment> & seg);
			std::shared_ptr<roadsegment> advanceRoadCandidate(roadsegment & seg, float additional_angle = 0.0f);
			
			// local constraint functions
			bool applyLocalContraintsToSegmentCandidate(std::shared_ptr<roadsegment> & seg_candidate);
			bool connectToDeadEndInRadius(std::shared_ptr<roadsegment>& seg_candidate, float radius);
			bool connectToCandidateStartInRadius(std::shared_ptr<roadsegment>& seg_candidate, float radius, float angle_tolerance);
			bool connectToExistingRoadSeg_intersecting(std::shared_ptr<roadsegment>& seg_candidate);
			bool connectToExistingRoadSeg_extending(std::shared_ptr<roadsegment>& seg_candidate, float radius);
			bool connectToExistingCrossing(std::shared_ptr<roadsegment>& seg_candidate, float radius);

			// helper functions
			bool insertCrossingAtExistingRoadSegment(std::shared_ptr<roadsegment> & existing_seg, std::shared_ptr<roadsegment> & seg, sf::Vector2f intersection);
			road_crossing* checkForCrossingInRadius(sf::Vector2f & pt, float radius);
			std::shared_ptr<roadsegment> checkForIntersection(roadsegment & seg, sf::Vector2f & intersecting_pt);
			std::shared_ptr<roadsegment> extendSegmentOntoExistingRoadSegment(roadsegment & seg, float maxdist, sf::Vector2f & intersecting_pt);

			cgSettings settings_;
			c2v mid_;

			std::list<std::shared_ptr<roadsegment>> road_candidates_;
		};
	}
}