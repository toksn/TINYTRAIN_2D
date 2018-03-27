#include "CityGenerator.h"
#include "tgfdefines.h"
#include <random>

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

		void CityGenerator::processRoadSegment(roadsegment_candidate& seg)
		{
			// check wether placement is possible
			bool valid_placement = true;

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