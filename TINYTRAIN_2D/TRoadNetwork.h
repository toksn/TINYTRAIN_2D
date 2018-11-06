#pragma once
#include "graph_tgf.h"
#include "tgfdefines.h"

namespace tinytrain
{
	struct road_connection_info
	{
		// absolute waypoints at the moment
		std::vector<sf::Vector2f> waypoints;
		struct stopping_info
		{
			// should probably be a [0,1] value in relation to the waypoints list
			float stop_at_dist;
			std::vector<sf::FloatRect> areas_to_check_before_continue;
			bool valid = false;
		};
		stopping_info stopinfo;
		stopping_info rotatedstopinfo;
	};
	struct edge_info : road_connection_info
	{
		direction out_slot;		// slot on the start crossing/node
		direction in_slot;		// slot on the destination crossing/node
	};
	struct inner_cross_connection_info : road_connection_info
	{
		float distance;
	};

	class ICrossingUser
	{
	public:
		virtual void startCrossing(int node_id, direction from, direction to) = 0;
	};

	using graph = tgf::graph::node_edgelist_graph<int, float, sf::FloatRect, edge_info>;
	class road_network
	{
		struct crossing_entry
		{
			ICrossingUser* user;
			direction from;
			direction to;
			bool fromQuadReleased = false;

			crossing_entry() {};

			crossing_entry(ICrossingUser* u, direction f, direction t)
			{
				user = u;
				from = f;
				to = t;
				fromQuadReleased = false;
			}
		};

		struct crossing_info
		{
			std::vector<crossing_entry> waiting;
			std::vector<crossing_entry> running;

			bool crossingQuadrants_inUse[direction::DIR_COUNT] = { false };
		};

	public:
		graph road_graph;
		inner_cross_connection_info crossing_connection_table[direction::DIR_COUNT][direction::DIR_COUNT];

		void update(float deltaTime);

		void applyToCrossing(ICrossingUser* user, int node_id, direction from, direction to);
		void removeFromCrossing(ICrossingUser* user, int node_id);
		void updateCrossingProgression(ICrossingUser* user, int node_id, float progression);

		std::vector<int> deadends;
	private:
		std::map<int, crossing_info> crossing_usage;
	};
}