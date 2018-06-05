#pragma once
#include <vector>

namespace tgf
{
	namespace graph
	{
		struct no_data {};
		template <class node_type = int, class dist_type = float, class node_data = no_data, class edge_data = no_data, class node_compare = std::less<node_type>> class node_edgelist_graph
		{
		public:
			struct node;
			struct edge
			{
				node_type source_node_;
				node_type target_node_;
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

			std::map<node_type, node, node_compare> nodes_;

			void addEdge(node_type from, node_type to, dist_type dist, edge_data data = edge_data())
			{
				edge e;
				e.user_data_ = data;
				e.distance_ = dist;

				auto temp = nodes_[to];
				e.source_node_ = from;
				e.target_node_ = to;

				nodes_[from].edges_.push_back(e);
			};

			void addNode(node_type id, node_data data = node_data())
			{
				nodes_[id].user_data_ = data;
			};

			// create shortest path table for all nodes
			//void init() {};
			//std::vector<node*> getShortestPath(node* start, node* end);

		private:
			//void djikstra(node* start, node* end = nullptr);
			//void djikstra_for_all_nodes();
		};
	}
}