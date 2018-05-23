#pragma once
#include <memory>
#include "Entity.h"
#include "TRailTrack.h"
#include "CityGenerator.h"

#include "graph_tgf.h"
using graph = tgf::graph::node_edgelist_graph<int, float>;

namespace tgf
{
	namespace utilities
	{
		class TextureAtlas;
	}
}

namespace tinytrain
{
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
		graph road_network_;

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

		sf::VertexArray background_static;
		sf::VertexArray foreground_static;
		sf::VertexArray foreground_dynamic;
	};
}