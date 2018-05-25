#include "TLevel_Builder.h"
#include "SplineTexture.h"
#include "GameState_Running.h"

// todo: maybe move into gamestate_running?
#define background_size_factor 1.0f;

namespace tinytrain
{
	TLevel_Builder::TLevel_Builder(GameState_Running * gs)
	{
		gs_ = gs;
		if (gs_ && gs_->game_)
		{
			texture_atlas_ = gs_->game_->getTextureAtlas();

			if (texture_atlas_)
				road_texture_width_ = texture_atlas_->getArea("road").width;
		}
	}

	TLevel_Builder::~TLevel_Builder()
	{
	}
		
	// this is used to generate:
	//		- tile type infos
	//
	//		- background of the tile (texture)
	//		- foreground of the tile (something special like a bridge, roofs of houses ect)
	//		- obstacles from collision textures
	//
	//		- road_network				
	//
	//		- possible randomly placed obstacles like trees
	std::unique_ptr<TLevel> TLevel_Builder::generateLevel_fromImage(sf::Image& map)
	{
		std::unique_ptr<TLevel> level = std::make_unique<TLevel>(gs_);

		// collect texture rects for every type from atlas (by name)
		if(texture_rects_by_tiletype_.size() == 0)
			texture_rects_by_tiletype_ = generateTileTypeInfos(texture_atlas_);
		
		// every pixel is an area of the size of a (simple) street
		const auto size = map.getSize();
		int tilesize = road_texture_width_ * background_size_factor;

		for (int x = 0; x < size.x; x++)
		{
			for (int y = 0; y < size.y; y++)
			{
				sf::Color col = map.getPixel(x, y);
				sf::IntRect curTileRect(x*tilesize, y*tilesize, tilesize, tilesize);

				//auto it = texture_rects_by_tiletype.find(col);
				//if (it != texture_rects_by_tiletype.end())
				auto cur_type_data = texture_rects_by_tiletype_[col.toInteger()];

				// generate background and collision
				if (cur_type_data.isValid)
				{	
					// common background layer
					addMapTile(level->background_static_, curTileRect, cur_type_data.common_bg, false);

					if (cur_type_data.tex_coords.size())
					{
						bool rotate = cur_type_data.rotationAllowed && rand() % 2;

						// get random element from the list
						int index = rand() % cur_type_data.tex_coords.size();
						auto iter = cur_type_data.tex_coords.begin();
						std::advance(iter, index);
						tile_type_info::texture_layer_set chosen_texture_set = iter->second;

						// add layers if there is any
						addMapTile(level->background_static_, curTileRect, chosen_texture_set.bg, rotate);
						addMapTile(level->foreground_static_, curTileRect, chosen_texture_set.fg, rotate);
						addMapTile(level->foreground_dynamic_, curTileRect, chosen_texture_set.fg_dyn, rotate);
						//...

						addCollision(curTileRect, chosen_texture_set.collision, texture_atlas_->getTexture(), rotate);
					}
				}
			}
		}

		// generate road tile and road network
		generateRoadNetwork_fromImage(map, level.get());

		// random yellow events (collectables, like passengers, construction_workers, bonus_points)
		// random target zones

		placeTrainTrack(level.get());

		return level;
	}

	std::unique_ptr<TLevel>  TLevel_Builder::generateLevel_random()
	{
		std::unique_ptr<TLevel> level = std::make_unique<TLevel>(gs_);

		tgf::utilities::CityGenerator city;
		tgf::utilities::cgSettings settings;

		float factor = 5.0f * background_size_factor;
		settings.road_crossingMinDist *= factor;
		settings.road_segLength *= factor;
		settings.road_chanceToSplitRadius *= factor;
		settings.road_chanceToContinueRadius *= factor;

		// variants for city generation:

		//// rectangular roads only
		//settings.road_segAngleRange = 0;
		//
		//// larger city radius
		//settings.road_chanceToSplitRadius *= 5.0f;
		//settings.road_chanceToContinueRadius *= 5.0f;

		city.applySettings(settings);

		auto t1 = std::clock();
		city.generate();

		level->roads_debug_.clear();
		level->roads_debug_.setPrimitiveType(sf::PrimitiveType::Lines);
		for (auto& road : city.road_segments_)
		{
			level->roads_debug_.append(sf::Vertex(road->a, road->col_a));
			level->roads_debug_.append(sf::Vertex(road->b, road->col_b));
		}

		int time = std::clock() - t1;
		printf("road generation took %i ms. %zi segments placed making %fms per segment\n", time, city.road_segments_.size(), (float)time / (float)(city.road_segments_.size()));

		level->roads_ = triangulateRoadSegments(city);

		/************************************************************************/
		// TODO: passengers to pick up

		placeTrainTrack(level.get());
		return level;
	}

	std::unique_ptr<TLevel> TLevel_Builder::loadLevel(std::string & filename)
	{
		return std::unique_ptr<TLevel>();
	}

	// this function has to place a (train), railtrack, (target zone)
	void TLevel_Builder::placeTrainTrack(TLevel* level)
	{
		/************************************************
		minimum req to play a level
		************************************************/
		// random start location
		if (level)
		{
			level->railtrack_ = std::make_unique<TRailTrack>(gs_);
			level->train_ = std::make_unique<TTrain>(gs_);
			level->train_->play();

			level->railtrack_->append(sf::Vector2f(200.0f, 50.f));
			level->railtrack_->append(sf::Vector2f(200.0f, 100.f));
			level->railtrack_->append(sf::Vector2f(250.0f, 140.f));
			level->railtrack_->addLastControlPointToHistory();
			level->railtrack_->addTrain(level->train_.get());
			level->train_->initWagons(15);
		}//*/

		/*longer version

		// create train for the player
		train_ = std::make_unique<TTrain>(gs_);
		train_->play();

		// create a railtrack for the train
		railtrack_ = std::make_unique<TRailTrack>(gs_);

		railtrack_->append(sf::Vector2f(200.0f, 50.f));
		railtrack_->append(sf::Vector2f(200.0f, 100.f));
		railtrack_->append(sf::Vector2f(250.0f, 140.f));
		railtrack_->append(sf::Vector2f(150.0f, 180.f));
		railtrack_->append(sf::Vector2f(130.0f, 70.f));

		c2v start{ 150.0f, 180.f };
		c2v end{ 130.0f, 70.f };
		float dist = 10.0f;
		int angle_range = 20;
		angle_range *= 100;
		
		//c2v seg = c2Sub(end, start);
		//// 57.295779513 := rad to degre conversion (rad * 180.0/pi)
		//float angle = atan2(seg.y, seg.x) * RAD_TO_DEG;
		//
		//for (size_t i = 0; i < 10; i++)
		//{
		//angle += ((rand() % angle_range) - angle_range * 0.5f)/100.0f;
		//
		////lastPos.x += rand() % 200 - 100;
		////lastPos.y += rand() % 200 - 100;
		//
		////lastPos.x += rand() % 30;
		////lastPos.y += rand() % 30;
		//
		//start = end;
		//end.x += dist * cos(angle / RAD_TO_DEG);
		//end.y += dist * sin(angle / RAD_TO_DEG);
		//
		//railtrack_->append(sf::Vector2f(end.x, end.y));
		//}
		
		railtrack_->addLastControlPointToHistory();
		railtrack_->addTrain(train_.get());
		train_->initWagons(15);

		// create obstacles for the games to be lost
		auto zone = std::make_unique<TObstacle>(gs_, false);
		zone->drawable_->setPosition(+30.0f, +30.0f);
		zone->updateCollisionShape();



		// create temporary component by constructor to use in copy constructor
		tgf::components::InterpolateToPoint c(zone->getPosition(), zone->getPosition() + sf::Vector2f(50.f, 0.0f), 2.0f, tgf::components::MovementType::TwoWay, true, false);
		c.start();
		zone->addNewComponent<tgf::components::InterpolateToPoint>(c);
		obstacles_.push_back(std::move(zone));

		// create target zone for the game to be won
		auto target_zone = std::make_unique<TObstacle>(gs_, true);
		target_zone->setPosition(-30.0f, -30.0f);
		target_zone->updateCollisionShape();

		// change duration and line for the target_zone
		c.duration_ = 1.0f;
		c.setControlPoints(target_zone->getPosition(), target_zone->getPosition() + sf::Vector2f(-30.f, -30.0f));
		target_zone->addNewComponent<tgf::components::InterpolateToPoint>(c);

		// create movement component by variables
		//auto movement_comp = std::make_unique<tgf::components::InterpolateToPoint>();
		//movement_comp->setControlPoints(target_zone->getPosition(), target_zone->getPosition() + sf::Vector2f(-30.f, -30.0f));
		//movement_comp->duration_ = 1.0f;
		//movement_comp->type_ = tgf::components::MovementType::TwoWay;
		//movement_comp->repeat_ = false;
		//movement_comp->start();
		//target_zone->addComponent(std::move(movement_comp));

		obstacles_.push_back(std::move(target_zone));
		*/
	}

	std::map < sf::Uint32, tile_type_info> TLevel_Builder::generateTileTypeInfos(tgf::utilities::TextureAtlas* atlas)
	{
		// available common backgrounds
		sf::IntRect common_bg_water = atlas->getArea("water");
		sf::IntRect common_bg_grass = atlas->getArea("grass_bg_01");
		// ...						


		std::map< sf::Uint32, tile_type_info> info;

		tile_type_info& cur = info[tile_colors::park];
		cur.isValid = true;
		cur.common_bg = common_bg_grass;		

		info[tile_colors::forest] = tile_type_info(cur);
		info[tile_colors::road] = tile_type_info(cur);
		info[tile_colors::residental] = tile_type_info(cur);
		info[tile_colors::industrial] = tile_type_info(cur);

		info[tile_colors::water] = tile_type_info(cur);
		info[tile_colors::water].common_bg = common_bg_water;

		// residental search for "house" strings in atlas
		info[tile_colors::residental].fillFromAtlas(atlas, "house_");
		
		// industrial search for "industrial" strings in atlas
		info[tile_colors::industrial].fillFromAtlas(atlas, "house_"); //"industrial_"
		// ...			

		return info;
	}


	void TLevel_Builder::generateRoadNetwork_fromImage(sf::Image & map, TLevel* level)
	{
		if (level == nullptr)
			return;

		// every pixel is an area of the size of a (simple) street
		const auto size = map.getSize();
		int tilesize = road_texture_width_ * background_size_factor;

		sf::IntRect n4 = texture_atlas_->getArea("road-4way");

		// road 3way, not connected dir defines the name, default from texture: 'e'
		sf::IntRect n3_e = texture_atlas_->getArea("road-3way");
		sf::IntRect n3_n = n3_e;
		sf::IntRect n3_s = n3_n;
		sf::IntRect n3_w = n3_n;

		// road 2way curve, connected dirs define the name, default from texture: 'se'
		sf::IntRect n2_se = texture_atlas_->getArea("road-2way");
		sf::IntRect n2_sw = n2_se;
		sf::IntRect n2_ne = n2_se;
		sf::IntRect n2_nw = n2_se;
		
		// straight road, connected dirs define the name, default from texture: 'ns'
		sf::IntRect n_ns = texture_atlas_->getArea("road");
		sf::IntRect n_we = n_ns;

		// create connection table
		// 0 N, 1 E, 2 S, 3 W
		road_connection_info foo[4][4];
		// N>S
		foo[0][2].waypoints.emplace_back(1.0f / 3.0f, 0.0f);
		foo[0][2].waypoints.emplace_back(1.0f / 3.0f, 1.0f);
		
		// N>E
		// circle center (64,0) - radius 1/3
		// 10 steps for waypoint generation 180-270°		
		float radius = 2.0f / 3.0f * tilesize;
		float angle = 180.0f;
		const float step = 10.0f * DEG_TO_RAD;
		const c2v center{ 64.0f, 0.0f };
		foo[0][1].waypoints.emplace_back(tilesize-radius, 0.0f);
		for (int i = 1; i < 9; i++)
		{
			angle += step;
			c2v pt = tgf::math::MathHelper2D::calc_point_on_circle(radius, angle, center);
			foo[0][1].waypoints.emplace_back(pt.x, pt.y);
		}
		foo[0][1].waypoints.emplace_back(64.0f, radius);

		//foo[0][1].waypoints.emplace_back(1.0f / 3.0f, 0.0f);
		//	// ...more points + stopping_info
		//foo[0][1].waypoints.emplace_back(1.0f, 2.0f / 3.0f);
		
		// N>W
		// circle center 0,0 - radius 1/3
		// 10 steps for waypoint generation 0-90°		
		float radius = 1.0f / 3.0f * tilesize;
		float angle = 0.0f;
		const float step = 10.0f * DEG_TO_RAD;
		foo[0][3].waypoints.emplace_back(radius, 0.0f);
		for (int i = 1; i < 9; i ++)
		{
			angle += step;
			c2v pt = tgf::math::MathHelper2D::calc_point_on_circle(radius, angle);
			foo[0][3].waypoints.emplace_back(pt.x, pt.y);
		}
		foo[0][3].waypoints.emplace_back(0.0f, radius);


		// create rest by rotating the N > X variant counter-clock wise P(x,y) -> P'(y, tilesize - x)
		// S>E
		foo[2][1].waypoints.resize(foo[0][3].waypoints.size());
		foo[1][0].waypoints.resize(foo[0][3].waypoints.size());
		foo[3][1].waypoints.resize(foo[0][3].waypoints.size());
		// flip vertically and horizontally
		for (auto& wp : foo[2][1].waypoints)
		{
			wp.x = tilesize - wp.x;
			wp.y = tilesize - wp.y;
		}
		// E>N
		foo[1][0].waypoints = foo[0][3].waypoints;
		// flip horizontally and reverse
		for (auto& wp : foo[1][0].waypoints)
			wp.x = tilesize - wp.x;
		foo[1][0].waypoints.reverse();
		// W>S
		foo[3][1].waypoints = foo[0][3].waypoints;
		// flip vertically and reverse
		for (auto& wp : foo[3][1].waypoints)
			wp.y = tilesize - wp.y;
		foo[3][1].waypoints.reverse();

		// S>N
		foo[2][0].waypoints.emplace_back(2.0f / 3.0f, 1.0f);
		foo[2][0].waypoints.emplace_back(2.0f / 3.0f, 0.0f);
		// W>E
		foo[2][0].waypoints.emplace_back(0.0f, 2.0f / 3.0f);
		foo[2][0].waypoints.emplace_back(1.0f, 2.0f / 3.0f);
		// E>W
		foo[2][0].waypoints.emplace_back(0.0f, 2.0f / 3.0f);
		foo[2][0].waypoints.emplace_back(1.0f, 2.0f / 3.0f);



		for (int x = 0; x < size.x; x++)
		{
			for (int y = 0; y < size.y; y++)
			{
				sf::Color col = map.getPixel(x, y);
				if (col.toInteger() == tile_colors::road)
				{
					sf::IntRect curTileRect(x*tilesize, y*tilesize, tilesize, tilesize);

					// count road neighbours
					std::vector<sf::Vector2u> road_neighbors;
					std::vector<sf::Vector2u> other_neighbors;
					int neighbor_samecolor_count = gatherPixelNeighborInfo_sameColor(map, x, y, &road_neighbors, &other_neighbors, false);
					

					if (neighbor_samecolor_count == 4)
						addMapTile(level->background_static_, curTileRect, texture_atlas_->getArea("road-4way"));
					else if (neighbor_samecolor_count == 3)
					{
						if (other_neighbors.size() == 1)
						{
							auto rect = texture_atlas_->getArea("road-3way");
							bool rotate = false;
							// right check of the _NOT_road
							if (other_neighbors.front().x != x + 1)
							{
								if (other_neighbors.front().x == x - 1)
								{
									// mirror both axis
									rect.left += rect.width;
									rect.width *= -1.0f;

									rect.top += rect.height;
									rect.height *= -1.0f;
								}
								else
								{
									rotate = true;
									if (other_neighbors.front().y == y - 1)
									{
										// mirror vertically
										rect.left += rect.width;
										rect.width *= -1.0f;
									}
									else
									{
										// mirror horizontally
										rect.top += rect.height;
										rect.height *= -1.0f;
									}
								}
							}

							addMapTile(level->background_static_, curTileRect, rect, rotate);
						}


					}
					else
					{
						auto rect = texture_atlas_->getArea("road");
						bool rotate = false;
						if (neighbor_samecolor_count == 1)
						{
							for (auto& pos : road_neighbors)
							{	// check for left right
								if (pos.x != x)
									rotate = true;
							}
						}
						else if (neighbor_samecolor_count == 2)
						{
							if (road_neighbors[0].x == road_neighbors[1].x || road_neighbors[0].y == road_neighbors[1].y)
								rotate = road_neighbors[0].x != x;
							else
							{
								rect = texture_atlas_->getArea("road-2way");
								bool v_mirror = road_neighbors[0].x < x || road_neighbors[1].x < x;
								bool h_mirror = road_neighbors[0].y < y || road_neighbors[1].y < y;

								if (v_mirror)
								{
									// mirror both axis
									rect.left += rect.width;
									rect.width *= -1.0f;
								}
								if (h_mirror)
								{
									rect.top += rect.height;
									rect.height *= -1.0f;
								}
							}
						}

						addMapTile(level->background_static_, curTileRect, rect, rotate);
					}
				}
			}
		}
	}

	// returns the amount of neighbors with the same color as the input pixel.
	// coordinates of same and other colored neighbors can be saved into provided vectors of pixelcoords (sf::Vector2u) for easy further processing
	//		warning: other colored neighbor coordinates may be out of bounds of the given image
	// can be used in 4 or 8 neighbor mode
	// todo: can be exported as a static method to some tgf::utilities class
	//
	// return -1 in case of an error
	int TLevel_Builder::gatherPixelNeighborInfo_sameColor(const sf::Image & map, const int x, const int y, std::vector<sf::Vector2u>* same_neighbours, std::vector<sf::Vector2u>* other_neighbours, bool includeDiagonalNeighbors)
	{
		auto size = map.getSize();
		if (x < 0 || y < 0 || x >= size.x || y >= size.y)
			return -1;

		int samecolor_neighbor_count = 0;
		const sf::Uint32 col = map.getPixel(x, y).toInteger();

		// check right
		if (x + 1 < size.x && map.getPixel(x + 1, y).toInteger() == col)
		{
			samecolor_neighbor_count++;
			if(same_neighbours)
				same_neighbours->push_back(sf::Vector2u(x + 1, y));
		}			
		else if (other_neighbours)
			other_neighbours->push_back(sf::Vector2u(x + 1, y));

		// check left
		if (x - 1 >= 0 && map.getPixel(x - 1, y).toInteger() == col)
		{
			samecolor_neighbor_count++;
			if (same_neighbours)
				same_neighbours->push_back(sf::Vector2u(x - 1, y));
		}
		else if (other_neighbours)
			other_neighbours->push_back(sf::Vector2u(x - 1, y));

		// check top
		if (y - 1 >= 0 && map.getPixel(x, y - 1).toInteger() == col)
		{
			samecolor_neighbor_count++;
			if (same_neighbours)
				same_neighbours->push_back(sf::Vector2u(x, y - 1));
		}
		else if (other_neighbours)
			other_neighbours->push_back(sf::Vector2u(x, y - 1));

		// check bottom
		if (y + 1 < size.y && map.getPixel(x, y + 1).toInteger() == col)
		{
			samecolor_neighbor_count++;
			if (same_neighbours)
				same_neighbours->push_back(sf::Vector2u(x, y + 1));
		}
		else if (other_neighbours)
			other_neighbours->push_back(sf::Vector2u(x, y + 1));

		if (includeDiagonalNeighbors)
		{
			// check top right
			if (x + 1 < size.x && y - 1 >= 0 && map.getPixel(x + 1, y - 1).toInteger() == col)
			{
				samecolor_neighbor_count++;
				if (same_neighbours)
					same_neighbours->push_back(sf::Vector2u(x + 1, y - 1));
			}
			else if (other_neighbours)
				other_neighbours->push_back(sf::Vector2u(x + 1, y - 1));

			// check top left
			if (x - 1 >= 0 && y - 1 >= 0 && map.getPixel(x - 1, y - 1).toInteger() == col)
			{
				samecolor_neighbor_count++;
				if (same_neighbours)
					same_neighbours->push_back(sf::Vector2u(x - 1, y - 1));
			}
			else if (other_neighbours)
				other_neighbours->push_back(sf::Vector2u(x - 1, y - 1));

			// check bottom right
			if (x + 1 < size.x && y + 1 < size.y && map.getPixel(x + 1, y + 1).toInteger() == col)
			{
				samecolor_neighbor_count++;
				if (same_neighbours)
					same_neighbours->push_back(sf::Vector2u(x + 1, y + 1));
			}
			else if (other_neighbours)
				other_neighbours->push_back(sf::Vector2u(x + 1, y + 1));

			// check bottom left
			if (x - 1 >= 0 && y + 1 < size.y && map.getPixel(x - 1, y + 1).toInteger() == col)
			{
				samecolor_neighbor_count++;
				if (same_neighbours)
					same_neighbours->push_back(sf::Vector2u(x - 1, y + 1));
			}
			else if (other_neighbours)
				other_neighbours->push_back(sf::Vector2u(x - 1, y + 1));
		}

		return samecolor_neighbor_count;
	}

	// vertex arrays are supposed to use PrimitiveType::Quads
	void TLevel_Builder::addMapTile(sf::VertexArray & vertices, sf::IntRect tile_rect, sf::IntRect texture_rect, bool rotate)
	{
		int startindex = vertices.getVertexCount();

		// add four points for the vertex array
		vertices.resize(vertices.getVertexCount() + 4);

		vertices[startindex + 0].position = { (float)tile_rect.left, (float)tile_rect.top };
		vertices[startindex + 1].position = { (float)tile_rect.left + tile_rect.width, (float)tile_rect.top };
		vertices[startindex + 2].position = { (float)tile_rect.left + tile_rect.width, (float)tile_rect.top + tile_rect.height };
		vertices[startindex + 3].position = { (float)tile_rect.left, (float)tile_rect.top + tile_rect.height };

		// texure coordinates
		vertices[startindex + 0].texCoords = { (float)texture_rect.left, (float)texture_rect.top };
		vertices[startindex + 1].texCoords = { (float)texture_rect.left + texture_rect.width, (float)texture_rect.top };
		vertices[startindex + 2].texCoords = { (float)texture_rect.left + texture_rect.width, (float)texture_rect.top + texture_rect.height };
		vertices[startindex + 3].texCoords = { (float)texture_rect.left, (float)texture_rect.top + texture_rect.height };
			
		// rotate and mirror to keep the correct pixel art lighting 
		if(rotate)
		{
			// rotate by -90°(270°) and mirror horizontally
			// 0 1			-->	3 0			--> 0 3
			// 3 2				2 1				1 2
			vertices[startindex + 3].texCoords = vertices[startindex + 1].texCoords;
			vertices[startindex + 1].texCoords = { (float)texture_rect.left, (float)texture_rect.top + texture_rect.height };
		}
	}

	void TLevel_Builder::addCollision(sf::IntRect tile_rect, sf::IntRect collision_texture_data, sf::Texture * tex, bool rotate)
	{
		// circle the texture for black pixels
		// expand pixels until completed area is found

		// use area to create an obstacle (cPoly?)
	}
	
	sf::VertexArray TLevel_Builder::triangulateRoadSegments(tgf::utilities::CityGenerator& city)
	{
		sf::VertexArray triangles;
		triangles.setPrimitiveType(sf::PrimitiveType::Triangles);
		std::unique_ptr<sf::Texture> road_texture;

		if (texture_atlas_)
		{
			road_texture = std::make_unique<sf::Texture>();
			road_texture->loadFromImage(*texture_atlas_->getImage(), texture_atlas_->getArea("road"));
			//full_texture_ = texture_atlas_->getTexture();
		}
		else
			return triangles;

		// generate all crossing types (4 way * 1, 3way * 4 = 5 types) triangulations
		// or just 4 way, 3 way
		// should be enough to generate 2 triangles each ?!

		// use crossing templates (copy) on each crossing to fill triangles array at pos of crossing with given texture coords

		// generate road triangles from splines (roadsegments = controlpoints) between deadends/crossings
		//float streetwidth = 16.0f;
		float streetwidth = road_texture_width_ * background_size_factor;
		//float crossing_radius = streetwidth
		std::vector<sf::VertexArray> tris;


		printf("road triangulation begin: crossings count %zi\n", city.road_crossings_.size());

		//while (city.road_deadends_.size())
		while (city.road_crossings_.size() || city.road_deadends_.size())
		{
			tgf::utilities::SplineTexture spline;
			spline.spline_->interpolateControlPointEnds_ = true;
			spline.useSplineptsForTextureSplitting_ = false;
			spline.getTriangleData().setPrimitiveType(sf::PrimitiveType::Triangles);
			std::vector<sf::Vector2f> ctrlPts;
			sf::Vector2f lastCtrlPt;
			tgf::utilities::road_crossing startcrossing;
			if (city.road_deadends_.size())
			{
				ctrlPts.push_back(city.road_deadends_.back());
				city.road_deadends_.pop_back();
				lastCtrlPt = ctrlPts.back();
			}
			else if (city.road_crossings_.size())
			{
				auto cross_iter = std::find_if(city.road_crossings_.begin(), city.road_crossings_.end(), [](const tgf::utilities::road_crossing& cross) {return cross.roads == 1; });
				if (cross_iter != city.road_crossings_.end())
				{
					//ctrlPts.push_back(cross_iter->pt);
					lastCtrlPt = cross_iter->pt;
					startcrossing = *cross_iter;
					city.road_crossings_.erase(cross_iter);
				}
				else
				{
					//printf("road triangulation warning: failed to fill starting controlpoint (no crossing with 1 road left?)\n");
					lastCtrlPt = city.road_crossings_.back().pt;
					startcrossing = city.road_crossings_.back();
					//ctrlPts.push_back(city.road_crossings_.back().pt);
				}
			}


			auto it = std::find_if(city.road_segments_.begin(), city.road_segments_.end(), [&lastCtrlPt](std::shared_ptr<tgf::utilities::roadsegment>& road)
			{return (road->a == lastCtrlPt || road->b == lastCtrlPt); });

			// create crossing ctrlpts because we started at a crossing
			if (ctrlPts.empty() && it != city.road_segments_.end())
				triangulation_insertSplineCtrlPtsForSegmentAtCrossing((*it).get(), &startcrossing, ctrlPts, true);

			while (it != city.road_segments_.end())
			{
				sf::Vector2f next_pt = (*it)->b;
				if (next_pt == lastCtrlPt)
					next_pt = (*it)->a;

				// find crossings at begin and end of segment
				auto cross_iter = std::find_if(city.road_crossings_.begin(), city.road_crossings_.end(), [&lastCtrlPt](tgf::utilities::road_crossing& cross)
				{return cross.pt == lastCtrlPt; });

				if (cross_iter != city.road_crossings_.end())
				{
					cross_iter->roads--;
					if (cross_iter->roads < 1)
						city.road_crossings_.erase(cross_iter);
				}

				cross_iter = std::find_if(city.road_crossings_.begin(), city.road_crossings_.end(), [&next_pt](tgf::utilities::road_crossing& cross)
				{return cross.pt == next_pt; });

				// remove current roadsegment
				std::shared_ptr<tgf::utilities::roadsegment> temp = *it;
				city.road_segments_.erase(it);

				if (cross_iter != city.road_crossings_.end())
				{
					cross_iter->roads--;

					if (cross_iter->roads < 1)
						city.road_crossings_.erase(cross_iter);
					else
					{
						// stop at crossing instead of stopping when no further roadsegment was found
						it = city.road_segments_.end();

						triangulation_insertSplineCtrlPtsForSegmentAtCrossing(temp.get(), &*cross_iter, ctrlPts);
						continue;
					}
				}

				ctrlPts.push_back(next_pt);
				lastCtrlPt = next_pt;
				it = std::find_if(city.road_segments_.begin(), city.road_segments_.end(), [&next_pt](std::shared_ptr<tgf::utilities::roadsegment>& road)
				{return (road->a == next_pt || road->b == next_pt); });
			}

			if (ctrlPts.size())
			{
				auto deadend = std::find(city.road_deadends_.begin(), city.road_deadends_.end(), ctrlPts.back());
				if (deadend != city.road_deadends_.end())
					city.road_deadends_.erase(deadend);
			}

			spline.setTexture(road_texture.get());
			spline.width_ = streetwidth;

			//spline.spline_->setcontrolspots(ctrlPts);
			spline.spline_->appendControlPoints(ctrlPts);
			spline.createTrianglesFromSpline();
			auto t = spline.getTriangleData();
			if (t.getVertexCount())
			{
				t.resize(t.getVertexCount() - 2);
				tris.push_back(t);
			}
			else if (ctrlPts.size())
				printf("road triangluation failed (ctrlpt count %zi) for one deadend. pt: %f, %f\n", ctrlPts.size(), ctrlPts.front().x, ctrlPts.front().y);
			else
				printf("road triangluation failed (ctrlpt count 0)\n");
		}

		printf("road triangulation end: %zi road segments and %zi crossings left.\n", city.road_segments_.size(), city.road_crossings_.size());
		// fill in road triangles
		auto roadTexCoords = texture_atlas_->getArea("road");
		for (auto& t : tris)
		{
			for (int i = 0; i < t.getVertexCount(); i++)
			{
				t[i].texCoords.x += roadTexCoords.left;
				t[i].texCoords.y += roadTexCoords.top;
				triangles.append(t[i]);
			}
		}
		// fill in crossing triangles (to draw over roads)


		return triangles;
	}

	// insert extra control points when a roadsegment is part of a crossing
	bool TLevel_Builder::triangulation_insertSplineCtrlPtsForSegmentAtCrossing(tgf::utilities::roadsegment* seg, tgf::utilities::road_crossing* crossing, std::vector<sf::Vector2f>& ctrl_pts, bool start)
	{
		float angle = crossing->crossing_index_from_angle(seg) * 90.0f + crossing->angle;

		////////////////////////////////////////////
		// example case: start = false
		//
		//
		//		calc_pt_1		calc_pt_2		cross->pt
		//			x<----(dist)---x------------->x
		//		   /
		//		  /			<---x angle (180°) + cross.angle
		//		 /
		//	last_pt
		////////////////////////////////////////////
		float distance = road_texture_width_ * background_size_factor;	// = streetwidth
		sf::Vector2f calc_pt_1, calc_pt_2;
		calc_pt_1 = calc_pt_2 = crossing->pt;

		// convert to radians
		sf::Vector2f displacement;
		angle *= DEG_TO_RAD;
		displacement.x = distance * cosf(angle);
		displacement.y = distance * sinf(angle);
		calc_pt_1 += displacement;
		calc_pt_2 += displacement * 0.5f;

		if (start)
		{
			ctrl_pts.push_back(crossing->pt);
			ctrl_pts.push_back(calc_pt_2);
			ctrl_pts.push_back(calc_pt_1);
		}
		else
		{
			ctrl_pts.push_back(calc_pt_1);
			ctrl_pts.push_back(calc_pt_2);
			ctrl_pts.push_back(crossing->pt);
		}

		return true;
	}
	
	void tile_type_info::fillFromAtlas(tgf::utilities::TextureAtlas * atlas, const std::string & prefix)
	{
		const std::string col = "collision_";
		const std::string fg_dyn = "fg_dyn_";
		const std::string fg = "fg_";
		const std::string bg = "bg_";
		auto first = atlas->texture_coords_.lower_bound(prefix);
		auto second = atlas->texture_coords_.upper_bound(prefix + "zzz");
		if (first != atlas->texture_coords_.end() && second != atlas->texture_coords_.end())
		{
			for (auto it = first; it != second; ++it)
			{
				std::string suffix = it->first.substr(prefix.length());
				if (suffix.length() > bg.length() && suffix.compare(0, bg.length(), bg) == 0)
				{
					suffix = suffix.substr(bg.length());
					tex_coords[suffix].bg = it->second;
				}
				else if (suffix.length() > fg.length() && suffix.compare(0, fg.length(), fg) == 0)
				{
					suffix = suffix.substr(fg.length());
					tex_coords[suffix].fg = it->second;
				}
				else if (suffix.length() > fg_dyn.length() && suffix.compare(0, fg_dyn.length(), fg_dyn) == 0)
				{
					suffix = suffix.substr(fg_dyn.length());
					tex_coords[suffix].fg_dyn = it->second;
				}
				else if (suffix.length() > col.length() && suffix.compare(0, col.length(), col) == 0)
				{
					suffix = suffix.substr(col.length());
					tex_coords[suffix].collision = it->second;
				}
			}
		}
	}
}