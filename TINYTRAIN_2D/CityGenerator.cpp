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

			//road_segments_.setPrimitiveType(sf::PrimitiveType::Lines);
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
			road_deadends_.clear();

			generateRoads();
		}


		void CityGenerator::generateRoads()
		{
			roadsegment start;
			road_crossing start_crossing(settings_.road_startingPoint);
			start.a = settings_.road_startingPoint;
		
			start.b = start.a + sf::Vector2f(settings_.road_segLength, 0.0f);
			start.angle = 0.0f;
			road_candidates_.push_back(std::shared_ptr<roadsegment>(new roadsegment(start)));
			start_crossing.addRoad(road_candidates_.back());

			start.b = start.a - sf::Vector2f(settings_.road_segLength, 0.0f);
			start.angle = 180.0f;
			road_candidates_.push_back(std::shared_ptr<roadsegment>(new roadsegment(start)));
			start_crossing.addRoad(road_candidates_.back());

			start.b = start.a + sf::Vector2f(0.0f, settings_.road_segLength);
			start.angle = 90.0f;
			road_candidates_.push_back(std::shared_ptr<roadsegment>(new roadsegment(start)));
			start_crossing.addRoad(road_candidates_.back());

			start.b = start.a - sf::Vector2f(0.0f, settings_.road_segLength);
			start.angle = 270.0f;
			road_candidates_.push_back(std::shared_ptr<roadsegment>(new roadsegment(start)));
			start_crossing.addRoad(road_candidates_.back());

			road_crossings_.push_back(start_crossing);

			while (road_candidates_.size())
			{
				std::shared_ptr<roadsegment> possible_segment;
				std::swap(possible_segment, road_candidates_.front());
				road_candidates_.pop_front();

				processRoadSegment(possible_segment);
			}
		}

		// applying local contraints, to generate new crossings/connections:
		//		- check for intersections with existing roads
		//		- chekc for close existing crossings to connect to
		//		- check for close existing roads to extend to
		//		- check for close deadends
		//
		// return true when the candidate may be processed normally
		// return false when the candidate was processed by local constraints already or is invalid to further process
		bool CityGenerator::applyLocalContraintsToSegmentCandidate(std::shared_ptr<roadsegment>& seg_candidate)
		{
			float close = settings_.road_segLength / 2.0f;
			//float close = settings_.road_crossingMinDist / 2.0f;
			if (connectToExistingCrossing(seg_candidate, close))
				return false;
			if (connectToExistingRoadSeg_intersecting(seg_candidate))
				return false;
			if (connectToCandidateStartInRadius(seg_candidate, close, 65.0f))
				return false;
			if (connectToDeadEndInRadius(seg_candidate, close))
				return false;
			if (connectToExistingRoadSeg_extending(seg_candidate, close))
				return false;
			
			
			return true;
		}
		
		std::shared_ptr<roadsegment> CityGenerator::checkForIntersection(roadsegment& seg, sf::Vector2f& intersecting_pt)
		{
			c2v s1_a, s1_b, s2_a, s2_b, intersection;
			s1_a = { seg.a.x, seg.a.y };
			s1_b = { seg.b.x, seg.b.y };
			// check for intersections
			for(auto& existing_road : road_segments_)
			{
				s2_a = { existing_road->a.x, existing_road->a.y };
				s2_b = { existing_road->b.x, existing_road->b.y };

				// ignore intersections when the points match exactly (they are probably from the same crossing)
				// todo: check if they are from the same crossing before continue
				if (c2Equal(s1_a, s2_a) || c2Equal(s1_a, s2_b) || c2Equal(s1_b, s2_a) || c2Equal(s1_b, s2_b))
					continue;

				//if (MathHelper2D::segment_segment_intersect(s1_a, s1_b, s2_a, s2_b))
				if (MathHelper2D::segment_segment_intersection(s1_a, s1_b, s2_a, s2_b, &intersection) > 0)
				{
					intersecting_pt = { intersection.x, intersection.y };
					return std::shared_ptr<roadsegment>(existing_road);
					//return existing_road;
				}
			}

			// no intersection found
			return nullptr;
		}

		std::shared_ptr<roadsegment> CityGenerator::extendSegmentOntoExistingRoadSegment(roadsegment& seg, float maxdist, sf::Vector2f& intersecting_pt)
		{
			c2v s1_a, s1_b, s2_a, s2_b, intersection;
			s1_a = { seg.a.x, seg.a.y };
			s1_b = { seg.b.x, seg.b.y };
			c2v dir = c2Sub(s1_b, s1_a);

			// check for intersections
			for(auto& road:road_segments_)
			{
				s2_a = { road->a.x, road->a.y };
				s2_b = { road->b.x, road->b.y };

				if (MathHelper2D::ray_to_segment_intersection(s1_a, dir, s2_a, s2_b, &intersection))
				{
					if (c2Distance(s1_b, intersection) < maxdist)
					{
						// todo: continue to find closest
						intersecting_pt = { intersection.x, intersection.y };
						return road;
					}
				}
			}

			return nullptr;
		}

		void CityGenerator::processRoadSegment(std::shared_ptr<roadsegment>& seg)
		{
			// check wether placement is possible
			bool valid_placement = applyLocalContraintsToSegmentCandidate(seg);

			if (valid_placement)
			{
				// add road candidate to road_segments
				road_segments_.push_back(seg);
				
				// calculate chances to split and continue
				float distance = c2Len(c2Sub({ seg->b.x, seg->b.y }, mid_));
				float chance_to_split = 1.0f - distance / settings_.road_chanceToSplitRadius;
				float chance_to_continue = 1.0f - distance / settings_.road_chanceToContinueRadius;

				if (chance_to_split < 0.0f)
					chance_to_split = 0.0f;
				if (chance_to_continue < 0.0f)
					chance_to_continue = 0.0f;

				//std::uniform_real_distribution<> random_dis(0.0, 1.0);
				float do_split = rand() / (double)RAND_MAX;
				float do_continue = rand() / (double)RAND_MAX;
				
				road_crossing cross(seg->b);
				cross.addRoad(seg);

				// check to continue (straight extension)
				if (do_continue < chance_to_continue)
				{
					auto next = advanceRoadCandidate(*seg);
					if (cross.addRoad(next) != -1)
						road_candidates_.push_back(next);
				}

				//road_candidates_.back();

				// check to split (left right extension)
				if (do_split < chance_to_split)
				{
					if (checkForCrossingInRadius(seg->b, settings_.road_crossingMinDist) == nullptr)
					{
						// splitting road may only create one new candidate (random direction) (+1 or -1)
						int direction_first = (rand() % 2) * 2 - 1;
						int direction_second = (rand() % 2) * 2 - 1;

						auto next = advanceRoadCandidate(*seg, 90.0f * direction_first);
						if (cross.addRoad(next) != -1)
							road_candidates_.push_back(next);
						
						if (direction_first != direction_second)
						{
							next = advanceRoadCandidate(*seg, 90.0f * direction_first);
							if (cross.addRoad(next) != -1)
								road_candidates_.push_back(next);
						}

						if (cross.roads > 2)
							road_crossings_.push_back(cross);
					}
				}

				if (cross.roads == 1)
					road_deadends_.push_back(seg->b);
			}
		}
		
		std::shared_ptr<roadsegment> CityGenerator::advanceRoadCandidate(roadsegment& seg, float additional_angle)
		{
			// copy current segment
			std::shared_ptr<roadsegment> nextsegment(new roadsegment(seg));
			// advance start point
			nextsegment->a = seg.b;

			// additionalangle: used for splitting the road, scramble const angle
			if (additional_angle != 0.0f)
			{
				nextsegment->angle += additional_angle;

				// randomize const angle decision
				nextsegment->constAngle = rand() % 2;
				nextsegment->constAngle *= (rand() / (double)RAND_MAX - 0.5f) * (float)settings_.road_segAngleRange;
				nextsegment->angle += nextsegment->constAngle;
			}
			else if (seg.constAngle == 0.0f)
				nextsegment->angle += (rand() / (double)RAND_MAX - 0.5f) * (float)settings_.road_segAngleRange;
			else
				nextsegment->angle += nextsegment->constAngle;


			// advance end point with new angle
			nextsegment->b.x += settings_.road_segLength * cos(nextsegment->angle * DEG_TO_RAD);
			nextsegment->b.y += settings_.road_segLength * sin(nextsegment->angle * DEG_TO_RAD);

			//road_candidates_.push_back(nextsegment);

			return std::move(nextsegment);
		}

		road_crossing* CityGenerator::checkForCrossingInRadius(sf::Vector2f& pt, float radius)
		{
			for (auto& c : road_crossings_)
			{
				if (c2Len(c2Sub({ pt.x, pt.y }, { c.pt.x, c.pt.y })) < radius)
				{
					return &c;
				}
			}

			return nullptr;
		}

		bool CityGenerator::insertCrossingAtExistingRoadSegment(std::shared_ptr<roadsegment>& existing_seg, std::shared_ptr<roadsegment>& seg, sf::Vector2f intersection)
		{
			//							x seg.b
			//						   /
			// existing_seg.a --------x--------- existing_seg.b (additionalsegment)
			//						 /
			//						/
			//					   x seg.a

			// copy existing segment to split it in two parts
			std::shared_ptr<roadsegment> additionalsegment = std::shared_ptr<roadsegment>(new roadsegment(*existing_seg));
			// save points to revert changes on failure
			sf::Vector2f seg_b = seg->b;
			sf::Vector2f existing_seg_b = existing_seg->b;

			//apply intersection point changes 
			seg->b = existing_seg->b = additionalsegment->a = intersection;

			road_crossing new_cross(intersection);
			if (new_cross.addRoad(seg) < 0 || new_cross.addRoad(existing_seg) < 0 || new_cross.addRoad(additionalsegment) < 0)
			{
				// reset segment info
				seg->b = seg_b;
				existing_seg->b = existing_seg_b;
				return false;
			}
				
			// add second part of old segment
			road_segments_.push_back(additionalsegment);

			// add new candidate
			seg->col_b = sf::Color::Red;
			road_segments_.push_back(seg);

			// save crossing
			road_crossings_.push_back(new_cross);

			return true;
		}

		// check other candidate STARTING points and saved deadends that are close (to possibly connect two close loose line endings)
		bool CityGenerator::connectToDeadEndInRadius(std::shared_ptr<roadsegment>& seg_candidate, float radius)
		{
			c2v pt = { seg_candidate->b.x, seg_candidate->b.y };
			for (auto end = road_deadends_.begin(); end != road_deadends_.end(); ++end)
			{
				if (c2Distance({ (*end).x, (*end).y }, pt) < radius)
				{
					seg_candidate->b = *end;

					seg_candidate->col_a = seg_candidate->col_b = sf::Color::Magenta;
					road_segments_.push_back(seg_candidate);
					
					road_deadends_.erase(end);
					return true;
				}
			}

			return false;
		}

		bool CityGenerator::connectToCandidateStartInRadius(std::shared_ptr<roadsegment>& seg_candidate, float radius, float angle_tolerance)
		{
			c2v pt = { seg_candidate->b.x, seg_candidate->b.y };

			// search all starting points of other candidates
			for (auto iter = road_candidates_.begin(); iter != road_candidates_.end(); ++iter)
			{
				auto& candidate = *iter;
				//if (candidate.a != seg.a || candidate.b != seg.b || candidate.angle != seg.angle || candidate.constAngle != seg.constAngle)
				if (candidate->a == seg_candidate->a && candidate->b == seg_candidate->b)
					printf("error, same candidate\n");
				else
				{
					float dist = c2Distance({ candidate->a.x, candidate->a.y }, pt);
					if (dist < radius)
					{
						auto closest_candidate = iter;
						auto crossing_on_candidate = std::find_if(road_crossings_.begin(), road_crossings_.end(), [&candidate](road_crossing& cross) {return cross.pt == candidate->a; });

						if (crossing_on_candidate != road_crossings_.end())
						{
							c2v pt_a = { seg_candidate->a.x, seg_candidate->a.y };
							float mindist = c2Distance(c2V((*closest_candidate)->b.x, (*closest_candidate)->b.y), pt_a);

							// check for roadcandidates from a crossing -> choose the best candidate to replace
							++iter;
							while (iter != road_candidates_.end() && (*iter)->a == candidate->a)
							{
								dist = c2Distance(c2V((*iter)->b.x, (*iter)->b.y), pt_a);
								if (dist < mindist)
								{
									mindist = dist;
									closest_candidate = iter;
								}
								++iter;
							}

							// angle of new seg vs angle of closest candidate (should differ about 180° +- tolerance)
							c2v t = c2Sub(c2V(seg_candidate->b.x, seg_candidate->b.y), c2V(seg_candidate->a.x, seg_candidate->a.y));
							// todo: angle should be the same as seg.angle?!
							float angle = atan2(t.y, t.x) * RAD_TO_DEG;
							float angle_diff = fmod(c2Abs(angle - (*closest_candidate)->angle), 360.0f);
							bool in_range = c2Abs(angle_diff - 180.0f) < angle_tolerance;
							if (in_range)
							{
								crossing_on_candidate->removeRoad(*closest_candidate);
							}
							//else
							//	printf("kept deadend candidate\n");

							// try to add the new road
							sf::Vector2f seg_b = seg_candidate->b;
							seg_candidate->b = candidate->a;
							if (crossing_on_candidate->addRoad(seg_candidate) != -1)
							{
								if(in_range)
									road_candidates_.erase(closest_candidate);

								// do it
								seg_candidate->col_a = sf::Color::White;
								seg_candidate->col_b = sf::Color::Magenta;
								road_segments_.push_back(seg_candidate);
								
								return true;
							}
							else
							{
								seg_candidate->b = seg_b;
								// re-add the closest candidate
								crossing_on_candidate->addRoad(*closest_candidate);
								return false;
							}
								
						}
						// not a crossing yet
						else
						{
							// angle of new seg vs angle of closest candidate (should differ about 180° +- tolerance)
							c2v t = c2Sub(c2V(seg_candidate->b.x, seg_candidate->b.y), c2V(seg_candidate->a.x, seg_candidate->a.y));
							// todo: angle should be the same as seg.angle?!
							float angle = atan2(t.y, t.x) * RAD_TO_DEG;
							float angle_diff = fmod(c2Abs(angle - (*closest_candidate)->angle), 360.0f);

							road_crossing cross((*closest_candidate)->a);
							//find existing road_segment endings, leading to the potential crossing point
							for(auto& existing_road : road_segments_)
							{
								if (existing_road->b == (*closest_candidate)->a)
								{
									cross.addRoad(existing_road);
									break;
								}
							}

							bool in_range = c2Abs(angle_diff - 180.0f) < angle_tolerance;
							if (in_range == false)
								cross.addRoad(*closest_candidate);

							// try to add the new road
							sf::Vector2f seg_b = seg_candidate->b;
							seg_candidate->b = candidate->a;
							if (cross.addRoad(seg_candidate) != -1)
							{
								if (cross.roads > 2)
									road_crossings_.push_back(cross);
								// todo: no new crossing and not in correct range -> skip this connection
								//else if (in_range == false)
								//	return true;

								if (in_range)
									road_candidates_.erase(closest_candidate);

								// do it
								seg_candidate->col_a = sf::Color::White;
								seg_candidate->col_b = sf::Color::Magenta;
								road_segments_.push_back(seg_candidate);
									
								return true;
							}
							else
							{
								seg_candidate->b = seg_b;
								return false;
							}
						}
					}
				}
			}
			return false;
		}

		// check existing road segments for an intersection of the candidate
		bool CityGenerator::connectToExistingRoadSeg_intersecting(std::shared_ptr<roadsegment>& seg_candidate)
		{
			sf::Vector2f intersection;
			auto intersection_segment = checkForIntersection(*seg_candidate, intersection);
			if(intersection_segment)
			{
				// check wether a new crossing is possible
				if (checkForCrossingInRadius(intersection, settings_.road_crossingMinDist) != nullptr)
				{
					auto cross_iter = std::find_if(road_crossings_.begin(), road_crossings_.end(), [&seg_candidate](road_crossing& cross) {return cross.pt == seg_candidate->a; });
					if (cross_iter != road_crossings_.end())
					{
						cross_iter->removeRoad(seg_candidate);
						if (cross_iter->roads < 3)
							road_crossings_.erase(cross_iter);
					}
					else
						road_deadends_.push_back(seg_candidate->a);
				}
				else
					insertCrossingAtExistingRoadSegment(intersection_segment, seg_candidate, intersection);

				return true;
			}

			return false;
		}

		// check for existing road segments that are close by extending the current segment
		bool CityGenerator::connectToExistingRoadSeg_extending(std::shared_ptr<roadsegment>& seg_candidate, float radius)
		{
			sf::Vector2f intersection;
			auto intersection_segment = extendSegmentOntoExistingRoadSegment(*seg_candidate, radius, intersection);
			if (intersection_segment)
			{
				// check wether a new crossing is possible
				if (checkForCrossingInRadius(intersection, settings_.road_crossingMinDist) != nullptr)
				{
					auto cross_iter = std::find_if(road_crossings_.begin(), road_crossings_.end(), [&seg_candidate](road_crossing& cross) {return cross.pt == seg_candidate->a; });
					if (cross_iter != road_crossings_.end())
					{
						cross_iter->removeRoad(seg_candidate);
						if (cross_iter->roads < 3)
							road_crossings_.erase(cross_iter);
					}
					else
						road_deadends_.push_back(seg_candidate->a);
				}
				else
					insertCrossingAtExistingRoadSegment(intersection_segment, seg_candidate, intersection);

				return true;
			}

			return false;
		}

		// check existing road crossings that are close to simply connect seg.b to that point
		bool CityGenerator::connectToExistingCrossing(std::shared_ptr<roadsegment>& seg_candidate, float radius)
		{
			road_crossing* crossing = checkForCrossingInRadius(seg_candidate->b, radius);
			if(crossing)
			{
				sf::Vector2f seg_b = seg_candidate->b;
				seg_candidate->b = crossing->pt;
				if (crossing->roads < 4 && crossing->addRoad(seg_candidate) != -1)
				{
					// add new candidate
					seg_candidate->col_a = seg_candidate->col_b = sf::Color::Blue;
					road_segments_.push_back(seg_candidate);

					return true;
				}
				else
					seg_candidate->b = seg_b;
			}

			return false;
		}
	}
}