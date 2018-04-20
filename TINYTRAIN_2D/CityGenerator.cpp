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
			road_deadends_.clear();

			generateRoads();
		}


		void CityGenerator::generateRoads()
		{
			roadsegment_candidate start;
			road_crossing start_crossing(settings_.road_startingPoint);
			start.a = settings_.road_startingPoint;
		
			start.b = start.a + sf::Vector2f(settings_.road_segLength, 0.0f);
			start.angle = 0.0f;
			road_candidates_.push_back(start);
			start_crossing.addRoad(start.angle);

			start.b = start.a - sf::Vector2f(settings_.road_segLength, 0.0f);
			start.angle = 180.0f;
			road_candidates_.push_back(start);
			start_crossing.addRoad(start.angle);

			start.b = start.a + sf::Vector2f(0.0f, settings_.road_segLength);
			start.angle = 90.0f;
			road_candidates_.push_back(start);
			start_crossing.addRoad(start.angle);

			start.b = start.a - sf::Vector2f(0.0f, settings_.road_segLength);
			start.angle = 270.0f;
			road_candidates_.push_back(start);
			start_crossing.addRoad(start.angle);

			road_crossings_.push_back(start_crossing);

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
		//		- check for close deadends
		//
		// return true when the candidate may be processed normally
		// return false when the candidate was processed by local constraints already or is invalid to further process
		bool CityGenerator::applyLocalContraintsToSegmentCandidate(roadsegment_candidate& seg)
		{
			float close = settings_.road_segLength / 2.0f;
			//float close = settings_.road_crossingMinDist / 2.0f;
			if (connectToExistingRoadSeg_intersecting(seg))
				return false;
			if (connectToCandidateStartInRadius(seg, close, 65.0f))
				return false;
			if (connectToDeadEndInRadius(seg, close))
				return false;
			if (connectToExistingRoadSeg_extending(seg, close))
				return false;
			if (connectToExistingCrossing(seg, close))
				return false;
			
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

		int CityGenerator::extendSegmentOntoExistingRoadSegment(roadsegment_candidate& seg, float maxdist, sf::Vector2f& intersecting_pt)
		{
			c2v s1_a, s1_b, s2_a, s2_b, intersection;
			s1_a = { seg.a.x, seg.a.y };
			s1_b = { seg.b.x, seg.b.y };
			c2v dir = c2Sub(s1_b, s1_a);

			// check for intersections
			for (int i = 0; i < road_segments_.getVertexCount(); i += 2)
			{
				s2_a = { road_segments_[i].position.x, road_segments_[i].position.y };
				s2_b = { road_segments_[i + 1].position.x, road_segments_[i + 1].position.y };

				if (MathHelper2D::ray_to_segment_intersection(s1_a, dir, s2_a, s2_b, &intersection))
				{
					if (c2Distance(s1_b, intersection) < maxdist)
					{
						// todo: continue to find closest
						intersecting_pt = { intersection.x, intersection.y };
						return i;
					}
				}
			}

			return -1;
		}

		void CityGenerator::processRoadSegment(roadsegment_candidate& seg)
		{
			// check wether placement is possible
			bool valid_placement = applyLocalContraintsToSegmentCandidate(seg);

			if (valid_placement)
			{
				// add road candidate to road_segments
				road_segments_.append(sf::Vertex(seg.a, sf::Color::Green));
				road_segments_.append(sf::Vertex(seg.b, sf::Color::Green));


				// calculate chances to split and continue
				float distance = c2Len(c2Sub({ seg.b.x, seg.b.y }, mid_));
				float chance_to_split = 1.0f - distance / settings_.road_chanceToSplitRadius;
				float chance_to_continue = 1.0f - distance / settings_.road_chanceToContinueRadius;

				if (chance_to_split < 0.0f)
					chance_to_split = 0.0f;
				if (chance_to_continue < 0.0f)
					chance_to_continue = 0.0f;

				//std::uniform_real_distribution<> random_dis(0.0, 1.0);
				float do_split = rand() / (double)RAND_MAX;
				float do_continue = rand() / (double)RAND_MAX;
				
				road_crossing cross(seg.b);
				cross.addRoad(seg.angle - 180.0f);

				// check to continue (straight extension)
				if (do_continue < chance_to_continue)
				{
					auto next = advanceRoadCandidate(seg);
					if (cross.addRoad(next.angle) != -1)
						road_candidates_.push_back(next);					
				}

				// check to split (left right extension)
				if (do_split < chance_to_split)
				{
					if (checkForCrossingInRadius(seg.b, settings_.road_crossingMinDist) == false)
					{
						// splitting road may only create one new candidate (random direction) (+1 or -1)
						int direction_first = (rand() % 2) * 2 - 1;
						int direction_second = (rand() % 2) * 2 - 1;

						auto next = advanceRoadCandidate(seg, 90.0f * direction_first);
						if (cross.addRoad(next.angle) != -1)
							road_candidates_.push_back(next);
						
						if (direction_first != direction_second)
						{
							next = advanceRoadCandidate(seg, 90.0f * direction_first);
							if (cross.addRoad(next.angle) != -1)
								road_candidates_.push_back(next);
						}

						if(cross.roads>2)
							road_crossings_.push_back(cross);
					}
				}

				if (cross.roads == 1)
					road_deadends_.push_back(seg.b);
			}
		}
		
		roadsegment_candidate CityGenerator::advanceRoadCandidate(roadsegment_candidate& seg, float additional_angle)
		{
			// copy current segment
			roadsegment_candidate nextsegment(seg);
			// advance start point
			nextsegment.a = seg.b;

			// additionalangle: used for splitting the road, scramble const angle
			if (additional_angle != 0.0f)
			{
				nextsegment.angle += additional_angle;

				// randomize const angle decision
				nextsegment.constAngle = rand() % 2;
				nextsegment.constAngle *= (rand() / (double)RAND_MAX - 0.5f) * (float)settings_.road_segAngleRange;
				nextsegment.angle += nextsegment.constAngle;
			}
			else if (seg.constAngle == 0.0f)
				nextsegment.angle += (rand() / (double)RAND_MAX - 0.5f) * (float)settings_.road_segAngleRange;
			else
				nextsegment.angle += nextsegment.constAngle;


			// advance end point with new angle
			nextsegment.b.x += settings_.road_segLength * cos(nextsegment.angle * DEG_TO_RAD);
			nextsegment.b.y += settings_.road_segLength * sin(nextsegment.angle * DEG_TO_RAD);

			//road_candidates_.push_back(nextsegment);

			return nextsegment;
		}

		bool CityGenerator::checkForCrossingInRadius(sf::Vector2f& pt, float radius, road_crossing* crossing)
		{
			for (auto& c : road_crossings_)
			{
				if (c2Len(c2Sub({ pt.x, pt.y }, { c.pt.x, c.pt.y })) < radius)
				{
					if (crossing)
						*crossing = c;

					return true;
				}
			}

			return false;
		}

		bool CityGenerator::insertCrossingAtExistingRoadSegment(int roadseg_startindex, roadsegment_candidate& seg, sf::Vector2f intersection)
		{
			//				  x seg.b
			//				 /
			// old_a -------x--------- old_b
			//				/
			//			   /
			//			  x seg.a

			road_crossing new_cross(intersection);
			c2v cross_pt = c2V(intersection.x, intersection.y);
			
			c2v t = c2Sub(c2V(seg.a.x, seg.a.y), cross_pt);
			float angle = atan2(t.y, t.x) * RAD_TO_DEG;
			if (new_cross.addRoad(angle) == -1)
				return false;

			t = c2Sub(c2V(road_segments_[roadseg_startindex].position.x, road_segments_[roadseg_startindex].position.y), cross_pt);
			angle = atan2(t.y, t.x) * RAD_TO_DEG;
			if (new_cross.addRoad(angle) == -1)
				return false;

			t = c2Sub(c2V(road_segments_[roadseg_startindex].position.x, road_segments_[roadseg_startindex].position.y), cross_pt);
			angle = atan2(t.y, t.x) * RAD_TO_DEG;
			if (new_cross.addRoad(angle) == -1)
				return false;
						
			// split existing road segment at intersection, cut candidate at intersection
			sf::Vector2f oldseg_b = road_segments_[roadseg_startindex + 1].position;
			seg.b = road_segments_[roadseg_startindex + 1].position = intersection;

			// re-create second part of the old segment
			road_segments_.append(sf::Vertex(seg.b, sf::Color::Green));
			road_segments_.append(sf::Vertex(oldseg_b, sf::Color::Green));

			// add new candidate
			road_segments_.append(sf::Vertex(seg.a, sf::Color::Green));
			road_segments_.append(sf::Vertex(seg.b, sf::Color::Red));

			// save crossing
			road_crossings_.push_back(new_cross);
		}

		// check other candidate STARTING points and saved deadends that are close (to possibly connect two close loose line endings)
		bool CityGenerator::connectToDeadEndInRadius(roadsegment_candidate& seg, float radius)
		{
			//radius *= 1.5f;

			c2v pt = { seg.b.x, seg.b.y };

			//if (connectClosestCandidateInRadius(seg, radius))
			//	return true;

			/*
			// search all starting points of other candidates
			for (auto iter = road_candidates_.begin(); iter != road_candidates_.end(); ++iter)
			{
				auto candidate = *iter;
				//if (candidate.a != seg.a || candidate.b != seg.b || candidate.angle != seg.angle || candidate.constAngle != seg.constAngle)
				if (candidate.a == seg.a && candidate.b == seg.b)
					printf("error, same candidate\n");
				else
				//if(&candidate != &seg)
				{
					float dist = c2Distance({ candidate.a.x, candidate.a.y }, pt);
					if (dist < radius)
					{
						auto closest = iter;
						auto crossing_on_candidate = std::find_if(road_crossings_.begin(), road_crossings_.end(), [&candidate](road_crossing& cross) {return cross.pt == candidate.a; });
						
						if (crossing_on_candidate != road_crossings_.end())
						{
							c2v pt_a = { seg.a.x, seg.a.y };
							float mindist = c2Distance(c2V(closest->b.x, closest->b.y), pt_a);

							// check for roadcandidates from a crossing -> choose the best candidate to replace
							++iter;
							while (iter != road_candidates_.end() && iter->a == candidate.a)
							{
								dist = c2Distance(c2V(iter->b.x, iter->b.y), pt_a);
								if (dist < mindist)
								{
									mindist = dist;
									closest = iter;
								}
								++iter;
							}
						}
						
						int road_count = 2;
						// angle of new seg vs angle of closest candidate (should differ about 180° +- tolerance (65 in this case))
						c2v t = c2Sub(c2V(seg.b.x, seg.b.y), c2V(seg.a.x, seg.a.y));

						float angle = atan2(t.y, t.x) * RAD_TO_DEG;
						float angle_diff = fmod(c2Abs(angle - closest->angle), 360.0f);
						if (c2Abs(angle_diff - 180.0f) < 65.0f)
						{
							//if(c2Abs(c2Abs(angle - closest->angle) - 180.0f)<65.0f)
							//if(c2Abs(angle-closest->angle) > 140.0f && c2Abs(angle - closest->angle) < 220.0f)
							
							if (crossing_on_candidate != road_crossings_.end())
							{
								crossing_on_candidate->removeRoad(closest.angle);
							}
							road_candidates_.erase(closest);
						}
						else
						{
							
							road_count++;
							printf("kept deadend candidate\n");
						}

						bool do_connect = false;
						if (crossing_on_candidate != road_crossings_.end())
						{	
							// existing crossing may stay the same or advance by one road
							road_count = crossing_on_candidate->roads + (road_count - 2);
							
						}

						if (road_count <= 4)
						{
							// do it
							seg.b = candidate.a;
							road_segments_.append(sf::Vertex(seg.a, sf::Color::White));
							road_segments_.append(sf::Vertex(seg.b, sf::Color::Magenta));

							if (crossing_on_candidate != road_crossings_.end())
								crossing_on_candidate->roads = road_count;
							else if (road_count>2)
								road_crossings_.push_back(road_crossing(seg.b, road_count));

							return true;
						}
					}
				}
			}
			*/
			for (auto end = road_deadends_.begin(); end != road_deadends_.end(); ++end)
			{
				if (c2Distance({ (*end).x, (*end).y }, pt) < radius)
				{
					seg.b = *end;

					road_segments_.append(sf::Vertex(seg.a, sf::Color::Magenta));
					road_segments_.append(sf::Vertex(seg.b, sf::Color::Magenta));

					road_deadends_.erase(end);
					return true;
				}
			}

			return false;
		}

		bool CityGenerator::connectToCandidateStartInRadius(roadsegment_candidate& seg, float radius, float angle_tolerance)
		{
			c2v pt = { seg.b.x, seg.b.y };

			// search all starting points of other candidates
			for (auto iter = road_candidates_.begin(); iter != road_candidates_.end(); ++iter)
			{
				auto candidate = *iter;
				//if (candidate.a != seg.a || candidate.b != seg.b || candidate.angle != seg.angle || candidate.constAngle != seg.constAngle)
				if (candidate.a == seg.a && candidate.b == seg.b)
					printf("error, same candidate\n");
				else
				{
					float dist = c2Distance({ candidate.a.x, candidate.a.y }, pt);
					if (dist < radius)
					{
						auto closest_candidate = iter;
						auto crossing_on_candidate = std::find_if(road_crossings_.begin(), road_crossings_.end(), [&candidate](road_crossing& cross) {return cross.pt == candidate.a; });

						if (crossing_on_candidate != road_crossings_.end())
						{
							c2v pt_a = { seg.a.x, seg.a.y };
							float mindist = c2Distance(c2V(closest_candidate->b.x, closest_candidate->b.y), pt_a);

							// check for roadcandidates from a crossing -> choose the best candidate to replace
							++iter;
							while (iter != road_candidates_.end() && iter->a == candidate.a)
							{
								dist = c2Distance(c2V(iter->b.x, iter->b.y), pt_a);
								if (dist < mindist)
								{
									mindist = dist;
									closest_candidate = iter;
								}
								++iter;
							}

							// angle of new seg vs angle of closest candidate (should differ about 180° +- tolerance)
							c2v t = c2Sub(c2V(seg.b.x, seg.b.y), c2V(seg.a.x, seg.a.y));
							// todo: angle should be the same as seg.angle?!
							float angle = atan2(t.y, t.x) * RAD_TO_DEG;
							float angle_diff = fmod(c2Abs(angle - closest_candidate->angle), 360.0f);
							bool in_range = c2Abs(angle_diff - 180.0f) < angle_tolerance;
							if (in_range)
							{
								crossing_on_candidate->removeRoad(closest_candidate->angle);
							}
							//else
							//	printf("kept deadend candidate\n");

							// try to add the new road
							if (crossing_on_candidate->addRoad(angle) != -1)
							{
								if(in_range)
									road_candidates_.erase(closest_candidate);

								// do it
								seg.b = candidate.a;
								road_segments_.append(sf::Vertex(seg.a, sf::Color::White));
								road_segments_.append(sf::Vertex(seg.b, sf::Color::Magenta));

								return true;
							}
							else
							{
								// re-add the closest candidate
								crossing_on_candidate->addRoad(closest_candidate->angle);
								return false;
							}
								
						}
						// not a crossing yet
						else
						{
							// angle of new seg vs angle of closest candidate (should differ about 180° +- tolerance)
							c2v t = c2Sub(c2V(seg.b.x, seg.b.y), c2V(seg.a.x, seg.a.y));
							// todo: angle should be the same as seg.angle?!
							float angle = atan2(t.y, t.x) * RAD_TO_DEG;
							float angle_diff = fmod(c2Abs(angle - closest_candidate->angle), 360.0f);

							road_crossing cross(closest_candidate->a);
							//find existing road_segment endings, leading to the potential crossing point
							for (int i = 1; i < road_segments_.getVertexCount(); i+=2)
							{
								if (road_segments_[i].position == closest_candidate->a)
								{
									c2v t = c2Sub(c2V(road_segments_[i-1].position.x, road_segments_[i-1].position.y), c2V(road_segments_[i].position.x, road_segments_[i].position.y));
									// todo: angle should be the same as seg.angle?!
									float angle_existing_segment = atan2(t.y, t.x) * RAD_TO_DEG;
									cross.addRoad(angle_existing_segment);
									break;
								}
							}

							bool in_range = c2Abs(angle_diff - 180.0f) < angle_tolerance;
							if (in_range == false)
								cross.addRoad(closest_candidate->angle);

							// try to add the new road
							if (cross.addRoad(angle) != -1)
							{
								if (in_range)
									road_candidates_.erase(closest_candidate);

								// do it
								seg.b = candidate.a;
								road_segments_.append(sf::Vertex(seg.a, sf::Color::White));
								road_segments_.append(sf::Vertex(seg.b, sf::Color::Magenta));

								if (cross.roads > 2)
									road_crossings_.push_back(cross);

								return true;
							}
							else
								return false;
						}
					}
				}
			}
			return false;
		}

		// check existing road segments for an intersection of the candidate
		bool CityGenerator::connectToExistingRoadSeg_intersecting(roadsegment_candidate& seg)
		{
			sf::Vector2f intersection;
			int i = checkForIntersection(seg, intersection);
			if (i >= 0)
			{
				// check wether a new crossing is possible
				if (checkForCrossingInRadius(intersection, settings_.road_crossingMinDist))
				{
					auto cross_iter = std::find_if(road_crossings_.begin(), road_crossings_.end(), [&seg](road_crossing& cross) {return cross.pt == seg.a; });
					if (cross_iter != road_crossings_.end())
					{
						cross_iter->removeRoad(seg.angle);
						if (cross_iter->roads < 3)
							road_crossings_.erase(cross_iter);
					}
					else
						road_deadends_.push_back(seg.a);
					//seg.b = crossing;
					//road_segments_.append(sf::Vertex(seg.a, sf::Color::Red));
					//road_segments_.append(sf::Vertex(seg.b, sf::Color::Green));
				}
				else
					return insertCrossingAtExistingRoadSegment(i, seg, intersection);

				return true;
			}

			return false;
		}

		// check for existing road segments that are close by extending the current segment
		bool CityGenerator::connectToExistingRoadSeg_extending(roadsegment_candidate& seg, float radius)
		{
			sf::Vector2f intersection;
			int i = extendSegmentOntoExistingRoadSegment(seg, radius, intersection);
			if (i >= 0)
			{
				// check wether a new crossing is possible
				if (checkForCrossingInRadius(intersection, settings_.road_crossingMinDist))
				{
					auto cross_iter = std::find_if(road_crossings_.begin(), road_crossings_.end(), [&seg](road_crossing& cross) {return cross.pt == seg.a; });
					if (cross_iter != road_crossings_.end())
					{
						cross_iter->removeRoad(seg.angle);
						if (cross_iter->roads < 3)
							road_crossings_.erase(cross_iter);
					}
					else
						road_deadends_.push_back(seg.a);
				}
				else
					return insertCrossingAtExistingRoadSegment(i, seg, intersection);

				return true;
			}

			return false;
		}

		// check existing road crossings that are close to simply connect seg.b to that point
		bool CityGenerator::connectToExistingCrossing(roadsegment_candidate& seg, float radius)
		{
			road_crossing crossing;
			if (checkForCrossingInRadius(seg.b, radius, &crossing))
			{
				if (crossing.roads < 4 && crossing.addRoad(seg.angle - 180.0f) != -1)
				{
					//todo: check for crossing to be full (4 pieces) already
					seg.b = crossing.pt;

					// add new candidate
					road_segments_.append(sf::Vertex(seg.a, sf::Color::Blue));
					road_segments_.append(sf::Vertex(seg.b, sf::Color::Blue));
					
					return true;
				}
			}

			return false;
		}
	}
}