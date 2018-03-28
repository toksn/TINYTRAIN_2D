#include "CityGenerator.h"
#include "tgfdefines.h"
#include <random>

using namespace tgf::math;

namespace tgf
{
	namespace utilities
	{
		CityGenerator::CityGenerator()
		{
			applySettings(settings_);

			road_segments_.setPrimitiveType(sf::PrimitiveType::Lines);
		}
		CityGenerator::~CityGenerator()
		{
		}

		void CityGenerator::applySettings(cgSettings settings)
		{
			settings_ = settings;

			mid_ = { settings_.road_startingPoint.x, settings_.road_startingPoint.y };
		}
		cgSettings CityGenerator::getSettings()
		{
			return settings_;
		}

		void CityGenerator::generate()
		{
			road_segments_.clear();
			road_candidates_.clear();
			road_crossings_.clear();

			generateRoads();
		}


		void CityGenerator::generateRoads()
		{
			roadsegment_candidate start;
			start.a = settings_.road_startingPoint;

			start.b = start.a + sf::Vector2f(settings_.road_segLength, 0.0f);
			start.angle = 0.0f;
			road_candidates_.push_back(start);

			start.b = start.a - sf::Vector2f(settings_.road_segLength, 0.0f);
			start.angle = 180.0f;
			road_candidates_.push_back(start);

			start.b = start.a + sf::Vector2f(0.0f, settings_.road_segLength);
			start.angle = 90.0f;
			road_candidates_.push_back(start);

			start.b = start.a - sf::Vector2f(0.0f, settings_.road_segLength);
			start.angle = 270.0f;
			road_candidates_.push_back(start);

			road_crossings_.push_back(settings_.road_startingPoint);

			while (road_candidates_.size())
			{
				auto possible_segment = road_candidates_.front();
				road_candidates_.pop_front();

				processRoadSegment(possible_segment);
			}
		}

		// applying local contraints, to generate new crossings/connections:
		//		- check for intersections with existing roads
		//		- chekc for close existing crossings to connect to
		//		- check for close existing roads to extend to
		//		- check for close starting points of other segment candidates to connect them
		//
		// return true when the candidate may be processed normally
		// return false when the candidate was processed by local constraints already or is invalid to further process
		bool CityGenerator::applyLocalContraintsToSegmentCandidate(roadsegment_candidate& seg)
		{
			// 1: check existing road segments for an intersection of the candidate
			sf::Vector2f intersection;
			int i = checkForIntersection(seg, intersection);
			if (i >= 0)
			{
				// check wether a new crossing is possible
				if (checkForCrossingInRadius(intersection, settings_.road_crossingMinDist) == false)
				{
					//create new 3 way crossing (oldseg.a > intersection; intersection > oldseg.b; seg.a > intersection)
					//createCrossingAtExistingRoadSegment(i, seg);
					//createCrossingAtExistingRoadSegment(i, seg.a, intersection);
					

					// split existing road segment at intersection, cut candidate at intersection
					sf::Vector2f oldseg_b = road_segments_[i + 1].position;
					seg.b = road_segments_[i + 1].position = intersection;

					// re-create second part of the old segment
					road_segments_.append(sf::Vertex(seg.b, sf::Color::Red));
					road_segments_.append(sf::Vertex(oldseg_b, sf::Color::Green));

					// add new candidate
					road_segments_.append(sf::Vertex(seg.a, sf::Color::Red));
					road_segments_.append(sf::Vertex(seg.b, sf::Color::Green));

					// save crossing
					road_crossings_.push_back(seg.b);
				}
				return false;
			}



			float close = settings_.road_segLength / 3.0f;
			// todo: 2: check existing road crossings that are close to simply connect seg.b to that point

			// todo: 3: check for existing road segments that are close
			// https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment

			// todo: 4: check other candidate STARTING points that are close (to possibly connect two close loose line endings)
			
			return true;
		}

		int CityGenerator::checkForIntersection(roadsegment_candidate& seg, sf::Vector2f& intersecting_pt)
		{
			c2v s1_a, s1_b, s2_a, s2_b, intersection;
			s1_a = { seg.a.x, seg.a.y };
			s1_b = { seg.b.x, seg.b.y };
			// check for intersections
			for (int i = 0; i < road_segments_.getVertexCount(); i += 2)
			{
				s2_a = { road_segments_[i].position.x, road_segments_[i].position.y };
				s2_b = { road_segments_[i + 1].position.x, road_segments_[i + 1].position.y };

				// ignore intersections when the points match exactly (they are probably from the same crossing)
				// todo: check if they are from the same crossing before continue
				if (c2Equal(s1_a, s2_a) || c2Equal(s1_a, s2_b) || c2Equal(s1_b, s2_a) || c2Equal(s1_b, s2_b))
					continue;

				//if (MathHelper2D::segment_segment_intersect(s1_a, s1_b, s2_a, s2_b))
				if (MathHelper2D::segment_segment_intersection(s1_a, s1_b, s2_a, s2_b, &intersection) > 0)
				{
					intersecting_pt = { intersection.x, intersection.y };
					return i;
				}
			}

			// no intersection found
			return -1;
		}

		void CityGenerator::processRoadSegment(roadsegment_candidate& seg)
		{
			// check wether placement is possible
			bool valid_placement = applyLocalContraintsToSegmentCandidate(seg);

			if (valid_placement)
			{
				// add road candidate to road_segments
				road_segments_.append(sf::Vertex(seg.a, sf::Color::Red));
				road_segments_.append(sf::Vertex(seg.b, sf::Color::Green));


				// calculate chances to split and continue
				float distance = c2Len(c2Sub({ seg.b.x, seg.b.y }, mid_));
				float chance_to_split = 1.0f - distance / settings_.road_chanceToSplitRadius;
				float chance_to_continue = 1.0f - distance / settings_.road_chanceToContinueRadius;

				if (chance_to_split < 0.0f)
					chance_to_split = 0.0f;
				if (chance_to_continue < 0.0f)
					chance_to_continue = 0.0f;

				std::uniform_real_distribution<> random_dis(0.0, 1.0);
				float do_split = rand() / (double)RAND_MAX;
				float do_continue = rand() / (double)RAND_MAX;
				
				// check to continue (straight extension)
				if (do_continue < chance_to_continue)
				{
					advanceRoadCandidate(seg);
				}

				// check to split (left right extension)
				if (do_split < chance_to_split)
				{
					if (checkForCrossingInRadius(seg.b, settings_.road_crossingMinDist) == false)
					{
						road_crossings_.push_back(seg.b);

						// splitting road may only create one new candidate (random direction)
						int direction_first = (rand() % 2) * 2 - 1;
						int direction_second = (rand() % 2) * 2 - 1;

						advanceRoadCandidate(seg, 90.0f * direction_first);
						if (direction_first != direction_second)
							advanceRoadCandidate(seg, 90.0f * direction_second);
					}
				}
			}
		}
		
		void CityGenerator::advanceRoadCandidate(roadsegment_candidate& seg, float additional_angle)
		{
			// copy current segment
			roadsegment_candidate nextsegment(seg);
			// advance start point
			nextsegment.a = seg.b;

			// additionalangle: used for splitting the road, scramble const angle
			if (additional_angle != 0.0f)
			{
				nextsegment.angle += additional_angle;

				// randomize const angle
				nextsegment.constAngle = rand() % 2;
				nextsegment.angle += (rand() / (double)RAND_MAX - 0.5f) * (float)settings_.road_segAngleRange;
			}
			else if (seg.constAngle == false)
				nextsegment.angle += (rand() / (double)RAND_MAX - 0.5f) * (float)settings_.road_segAngleRange;


			// advance end point with new angle
			nextsegment.b.x += settings_.road_segLength * cos(nextsegment.angle * DEG_TO_RAD);
			nextsegment.b.y += settings_.road_segLength * sin(nextsegment.angle * DEG_TO_RAD);

			road_candidates_.push_back(nextsegment);
		}

		bool CityGenerator::checkForCrossingInRadius(sf::Vector2f& pt, float radius)
		{
			for (auto& crossing : road_crossings_)
			{
				if (c2Len(c2Sub({ pt.x, pt.y }, { crossing.x, crossing.y })) < radius)
					return true;
			}

			return false;
		}
	}
}