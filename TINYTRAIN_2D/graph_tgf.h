#pragma once
#include <vector>

namespace tgf
{
	namespace graph
	{
		struct no_data {};
		template <class node_data = no_data, class edge_data = no_data> class node_edgelist_graph
		{
		public:
			struct node;
			struct edge
			{
				node* target_node_;
				float distance_;				
				edge_data user_data_;
			};
			struct node
			{
				std::vector<edge> edges_;
				node_data user_data_;
			};

		
			node_edgelist_graph();
			~node_edgelist_graph();

			std::vector<node*> nodes;

			// create shortest path table for all nodes
			init();
			std::vector<node*> getShortestPath(node* start, node* end);

		private:
			void djikstra(node* start, node* end = nullptr);
			void djikstra_for_all_nodes();
		};

		template<class node_data, class edge_data>
		inline node_edgelist_graph<node_data, edge_data>::node_edgelist_graph()
		{
		}

		template<class node_data, class edge_data>
		inline node_edgelist_graph<node_data, edge_data>::~node_edgelist_graph()
		{
		}

	}
}