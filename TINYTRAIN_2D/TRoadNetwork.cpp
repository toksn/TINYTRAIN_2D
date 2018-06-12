#include "TRoadNetwork.h"

namespace tinytrain
{
	void road_network::update(float deltaTime)
	{
		for (auto& c : crossing_usage)
		{
			if (c.second.waiting.size() && c.second.running.empty())
			{
				// todo: implement priority logic 
				auto pickedUserToStart = c.second.waiting.front();
				c.second.waiting.erase(c.second.waiting.begin());
				c.second.running.emplace_back(std::move(pickedUserToStart));

				
				c.second.running.front().user->startCrossing(c.first, pickedUserToStart.from, pickedUserToStart.to);
			}
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
	}
}
