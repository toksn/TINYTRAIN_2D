#include "TRoadNetwork.h"

namespace tinytrain
{
	void road_network::update(float deltaTime)
	{
		for (auto& c : crossing_usage)
		{
#if 0		// first in, last out
			if (c.second.waiting.size() && c.second.running.empty())
			{
				// todo: implement priority logic 
				auto pickedUserToStart = c.second.waiting.front();
				c.second.waiting.erase(c.second.waiting.begin());
				c.second.running.emplace_back(std::move(pickedUserToStart));

				c.second.running.front().user->startCrossing(c.first, pickedUserToStart.from, pickedUserToStart.to);
			}
#else			
			bool allQuadsUsed = c.second.crossingQuadrants_inUse[0] && c.second.crossingQuadrants_inUse[1] && c.second.crossingQuadrants_inUse[2] && c.second.crossingQuadrants_inUse[3];		
			
			if (allQuadsUsed == false)
			{
				for (auto waiter = c.second.waiting.begin(); waiter != c.second.waiting.end(); ++waiter)
				{
					// todo: add and link dev-doc image from canvas app
					// check if quadrants are in not already in use
					int from = waiter->from;
					if (c.second.crossingQuadrants_inUse[from] == false)
					{
						// to is rotated by one, may be the same quadrant as "from", so only one quadrant is used when turning right
						int to = (waiter->to + 1) % direction::DIR_COUNT;
						if (c.second.crossingQuadrants_inUse[to] == false)
						{
							// when exactly one is already running, we may have two diagonals, which could lead to crashes
							if (c.second.running.size() == 1)
							{
								// current waiter is diagonal (turn left) AND current only runner is diagonal as well
								auto& runner = c.second.running.front();
								if (((from + 2) % direction::DIR_COUNT) == to && ((runner.from+1)%direction::DIR_COUNT == runner.to))
									continue;
							}

							c.second.running.emplace_back(*waiter);
							c.second.crossingQuadrants_inUse[from] = true;
							c.second.crossingQuadrants_inUse[to] = true;
							
							// tell the crossing user to start
							waiter->user->startCrossing(c.first, waiter->from, waiter->to);

							// remove from waiting queue
							waiter = c.second.waiting.erase(waiter);
							if (waiter == c.second.waiting.end())
								break;
						}
					}
				}
			}	
#endif
		}
	}

	void road_network::applyToCrossing(ICrossingUser * user, int node_id, direction from, direction to)
	{
		//if (road_graph.nodes_[node_id].edges_.size() > 2)
			crossing_usage[node_id].waiting.emplace_back(user, from, to);
		//else if(user)
		//	user->startCrossing(node_id, from, to);
	}

	void road_network::removeFromCrossing(ICrossingUser * user, int node_id)
	{
		auto cross = crossing_usage.find(node_id);
		
		if (cross != crossing_usage.end() ) //&& road_graph.nodes_[node_id].edges_.size() > 2)
		{
			auto& runners = cross->second.running;
			for (auto it = runners.begin(); it != runners.end(); ++it)
			{
				if (it->user == user)
				{
					if (it->fromQuadReleased == false)
						cross->second.crossingQuadrants_inUse[it->from] = false;

					int to = (it->to + 1) % direction::DIR_COUNT;
					cross->second.crossingQuadrants_inUse[to] = false;

					runners.erase(it);
					return;
				}
			}
			auto& waiters = cross->second.waiting;
			for (auto it = waiters.begin(); it != waiters.end(); ++it)
			{
				if (it->user == user)
				{
					waiters.erase(it);
					return;
				}
			}
		}

		printf("remove from crossing failed. should not happen?\n");
	}
	
	void road_network::updateCrossingProgression(ICrossingUser * user, int node_id, float progression)
	{
		if (progression < 0.75f || user == nullptr)
			return;

		auto cross = crossing_usage.find(node_id);

		if (cross != crossing_usage.end())
		{
			for (auto it = cross->second.running.begin(); it != cross->second.running.end(); ++it)
			{
				if (it->user == user)
				{
					if (it->fromQuadReleased == false)
					{
						// only free the first quadrant, when it is not the same as the second quadrant
						int to = (it->to + 1) % direction::DIR_COUNT;
						if (to != it->from)
						{
							cross->second.crossingQuadrants_inUse[it->from] = false;
							it->fromQuadReleased = true;
						}
					}
					return;
				}
			}
		}
	}
}
