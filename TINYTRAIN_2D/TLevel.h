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

	// possible tile type colors
	namespace tile_colors
	{
		const sf::Uint32 water(sf::Color(0, 162, 232).toInteger());	//blue
		const sf::Uint32 residental(sf::Color(237, 28, 36).toInteger());	//red
		const sf::Uint32 park(sf::Color(181, 230, 29).toInteger());	//bright green
		const sf::Uint32 forest(sf::Color(34, 177, 76).toInteger());	//dark green
		const sf::Uint32 road(sf::Color(127, 127, 127).toInteger());	//grey
		const sf::Uint32 industrial(sf::Color(255, 201, 14).toInteger());	//yellow
	}

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
		std::map<std::string, texture_layer_set> tex_coords;
		bool rotationAllowed = true;
		bool isValid = false;

		void fillFromAtlas(tgf::utilities::TextureAtlas* atlas, const std::string & prefix);
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
		std::map < sf::Uint32, tile_type_info> generateTileTypeInfos(tgf::utilities::TextureAtlas * atlas);
		void addMapTile(sf::VertexArray& vertices, sf::IntRect tile_rect, sf::IntRect texture_rect, bool rotate = false);
		void addCollision(sf::IntRect tile_rect, sf::IntRect collision_texture_data, sf::Texture* tex, bool rotate = false);

		tgf::utilities::TextureAtlas* texture_atlas_ = nullptr;
		GameState_Running* gs_;
	};
}