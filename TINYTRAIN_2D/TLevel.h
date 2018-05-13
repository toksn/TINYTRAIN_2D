#pragma once
#include <memory>
#include "Entity.h"
#include "TRailTrack.h"
#include "CityGenerator.h"

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

	struct tile_type_info
	{
		struct texture_layer_set
		{
			sf::IntRect bg;
			sf::IntRect fg;
			sf::IntRect fg_dyn;
			sf::IntRect collision;
		};
		sf::IntRect common_bg;
		std::vector<texture_layer_set> tex_coords;
		bool rotationAllowed = true;
		bool isValid = false;
	};

	class TLevel : public tgf::Entity
	{
	public:
		TLevel(GameState_Running* gs);
		~TLevel();

		// load a level from file
		void load(std::string file = "");

		void restart();
				
		std::unique_ptr<TTrain> train_;
		std::unique_ptr<TRailTrack> railtrack_;
		std::vector<std::unique_ptr<TObstacle>> obstacles_;

	protected:
		// Inherited via Entity
		virtual void onDraw(sf::RenderTarget * target) override;
		virtual void onUpdate(float deltaTime) override;

		void onKeyPressed(sf::Event & e);

		sf::VertexArray background_static;
		sf::VertexArray foreground_static;
		sf::VertexArray foreground_dynamic;

		sf::VertexArray roads_;
		sf::VertexArray roads_debug_;
		bool drawDebug_;

		void generateLevel_random();
		sf::VertexArray triangulateRoadSegments(tgf::utilities::CityGenerator & city);
		bool triangulation_insertSplineCtrlPtsForSegmentAtCrossing(tgf::utilities::roadsegment* seg, tgf::utilities::road_crossing* crossing, std::vector<sf::Vector2f>& ctrl_pts, bool start = false);

		void generateLevel_fromImage(sf::Image & map);
		void addMapTile(sf::VertexArray& vertices, sf::IntRect tile_rect, sf::IntRect texture_rect, bool rotationAllowed = false);
		void addCollision(sf::IntRect tile_rect, sf::IntRect collision_texture_data, sf::Texture* tex);

		tgf::utilities::TextureAtlas* texture_atlas_ = nullptr;
		GameState_Running* gs_;
	};
}