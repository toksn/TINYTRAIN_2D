#pragma once

#include "CityGenerator.h"
#include "TextureAtlas.h"
#include "TLevel.h"
#include "CollisionManager.h"

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

		const sf::Uint32 trees(sf::Color(10, 10, 10, 10).toInteger());	// random color to store tree information (non-valid tile color)
	}

	struct tile_type_info
	{
		struct texture_layer_set
		{
			sf::IntRect bg;
			sf::IntRect fg;
			sf::IntRect fg_dyn;
			std::vector<c2AABB> collision_polys;
		};
		sf::IntRect common_bg;
		std::map<std::string, texture_layer_set> texture_layer_info;
		bool rotationAllowed = true;
		bool isValid = false;
		sf::Vector2u tree_count_range; // x = min, y = max

		void fillFromAtlas(tgf::utilities::TextureAtlas* atlas, const std::string & prefix);
		std::vector<c2AABB> extractCollisionPolys(sf::IntRect & collision_texture_data, sf::Image * img);
		std::vector<direction> detectBorder_clockwiseDirChain(sf::Image * img, const sf::Vector2u startCoords, int * checked, bool onlyConvex = false);
	};

	class TLevel_Builder
	{
		friend class tile_type_info;
	public:
		TLevel_Builder(GameState_Running* gs);
		~TLevel_Builder();

		std::unique_ptr<TLevel> generateLevel_random();
		std::unique_ptr<TLevel> generateLevel_fromImage(sf::Image & map);
		std::unique_ptr<TLevel> loadLevel(std::string& filename);

	protected:
		void placeTrainTrack(TLevel * level);

		sf::VertexArray triangulateRoadSegments(tgf::utilities::CityGenerator & city);
		bool triangulation_insertSplineCtrlPtsForSegmentAtCrossing(tgf::utilities::roadsegment* seg, tgf::utilities::road_crossing* crossing, std::vector<sf::Vector2f>& ctrl_pts, bool start = false);

		std::map < sf::Uint32, tile_type_info> generateTileTypeInfos(tgf::utilities::TextureAtlas * atlas);
		void generateRoadNetwork_fromImage(sf::Image & map, TLevel* level);
		int findNextRoadNode(int start_id, std::vector<direction>& edge_directions, sf::Image& map);
		static int gatherPixelNeighborInfo_sameColor(const sf::Image & map, const int x, const int y, std::vector<sf::Vector2u>* same_neighbours = nullptr, std::vector<sf::Vector2u>* other_neighbours = nullptr, bool includeDiagonalNeighbors = false);
		static int gatherPixelNeighborDirs_sameColor(const sf::Image & map, const int x, const int y, std::vector<direction>* same_neighbours = nullptr, std::vector<direction>* other_neighbours = nullptr);
		void initConnectionTable(road_network & network, float tilesize);
		void addMapTile(sf::VertexArray& vertices, sf::IntRect tile_rect, sf::IntRect texture_rect, int rectangular_rotation = 0, bool mirror_horizontally = false, bool mirror_vertically = false);
		void addCollision(TLevel * level, sf::IntRect tile_rect, std::vector<c2AABB>& collisions,   int rectangular_rotation = 0, bool mirror_horizontally = false, bool mirror_vertically = false);
		std::vector<sf::Vector2u> tryToPlaceTrees(const sf::IntRect & tilerect, const std::vector<c2AABB>& other_colliders, int tree_count);
		void plantTree(TLevel * level, const sf::Vector2u & pos, const sf::IntRect & tile_rect, const tile_type_info & tree_info);
		

		float road_texture_width_ = 32.0f;
		GameState_Running* gs_;
		tgf::utilities::TextureAtlas* texture_atlas_ = nullptr;
		std::map< sf::Uint32, tile_type_info> texture_rects_by_tiletype_;
	};
}