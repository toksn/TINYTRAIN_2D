#pragma once
#include <vector>

namespace tgf
{
	namespace graph
	{
		struct no_data {};
		template <class node_type = int, class dist_type = float, class node_data = no_data, class edge_data = no_data> class node_edgelist_graph
		{
		public:
			struct node;
			struct edge
			{
				node* target_node_;
				dist_type distance_;
				edge_data user_data_;
			};
			struct node
			{
				std::vector<edge> edges_;
				node_data user_data_;
			};

			node_edgelist_graph() {};
			~node_edgelist_graph() {};

			std::map<node_type, node> nodes_;

			void addEdge(node_type from, node_type to, dist_type dist, edge_data data = edge_data())
			{
				edge e;
				e.user_data_ = data;
				e.distance_ = dist;
				e.target_node_ = &nodes_[to];

				nodes_[from].edges_.push_back(e);
			};

			void addVertex(node_type id, node_data data = node_data())
			{
				nodes[id].user_data_ = data;
			};

			// create shortest path table for all nodes
			void init() {};
			std::vector<node*> getShortestPath(node* start, node* end);

		private:
			void djikstra(node* start, node* end = nullptr);
			void djikstra_for_all_nodes();
		};
	}
}