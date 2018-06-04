#pragma once
#include <memory>
#include "Entity.h"
#include "TRailTrack.h"
#include "CityGenerator.h"

#include "graph_tgf.h"

namespace tgf
{
	namespace utilities
	{
		class TextureAtlas;
	}
}

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
			std::vector<sf::IntRect> areas_to_check_before_continue;
		} stopinfo;
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

	using graph = tgf::graph::node_edgelist_graph<int, float, tgf::graph::no_data, edge_info>;
	struct road_network
	{
		graph road_graph;
		inner_cross_connection_info crossing_connection_table[direction::DIR_COUNT][direction::DIR_COUNT];
	};

	class TTrain;
	class TObstacle;
	class GameState_Running;	

	class TLevel : public tgf::Entity
	{
		friend class TLevel_Builder;

	public:
		TLevel(GameState_Running* gs);
		~TLevel();

		void restart_();
				
		std::unique_ptr<TTrain> train_;
		std::unique_ptr<TRailTrack> railtrack_;
		std::vector<std::unique_ptr<TObstacle>> obstacles_;
		road_network road_network_;

	protected:
		// Inherited via Entity
		virtual void onDraw(sf::RenderTarget * target) override;
		virtual void onUpdate(float deltaTime) override;

		void onKeyPressed(sf::Event & e);

		sf::VertexArray roads_;
		sf::VertexArray roads_debug_;
		bool drawDebug_;

		tgf::utilities::TextureAtlas* texture_atlas_ = nullptr;
		float road_texture_width_;
		GameState_Running* gs_;

		sf::VertexArray background_static_;
		sf::VertexArray foreground_static_;
		sf::VertexArray foreground_dynamic_;
	};
}