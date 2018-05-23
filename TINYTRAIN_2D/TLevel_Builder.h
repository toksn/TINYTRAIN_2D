#pragma once

#include "CityGenerator.h"
#include "TextureAtlas.h"
#include "TLevel.h"

namespace tinytrain
{
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

	class TLevel_Builder
	{
	public:
		TLevel_Builder(GameState_Running* gs);
		~TLevel_Builder();

		std::unique_ptr<TLevel> generateLevel_random();
		std::unique_ptr<TLevel> generateLevel_fromImage(sf::Image & map);
		std::unique_ptr<TLevel> loadLevel(std::string& filename);

	protected:
		sf::VertexArray triangulateRoadSegments(tgf::utilities::CityGenerator & city);
		bool triangulation_insertSplineCtrlPtsForSegmentAtCrossing(tgf::utilities::roadsegment* seg, tgf::utilities::road_crossing* crossing, std::vector<sf::Vector2f>& ctrl_pts, bool start = false);

		void placeTrainTrack(TLevel * level);

		std::map < sf::Uint32, tile_type_info> generateTileTypeInfos(tgf::utilities::TextureAtlas * atlas);
		void addMapTile(sf::VertexArray& vertices, sf::IntRect tile_rect, sf::IntRect texture_rect, bool rotate = false);
		void addCollision(sf::IntRect tile_rect, sf::IntRect collision_texture_data, sf::Texture* tex, bool rotate = false);


		float road_texture_width_ = 32.0f;
		GameState_Running* gs_;
		tgf::utilities::TextureAtlas* texture_atlas_ = nullptr;
		std::map< sf::Uint32, tile_type_info> texture_rects_by_tiletype_;
	};
}