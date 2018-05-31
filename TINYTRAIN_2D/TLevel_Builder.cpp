#include "TLevel_Builder.h"
#include "SplineTexture.h"
#include "GameState_Running.h"
#include <set>
//#include "tgfdefines.h"

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
						// get random element from the list
						int index = rand() % cur_type_data.tex_coords.size();
						auto iter = cur_type_data.tex_coords.begin();
						std::advance(iter, index);
						tile_type_info::texture_layer_set chosen_texture_set = iter->second;

						bool rotate_and_mirror = cur_type_data.rotationAllowed && rand() % 2;
						int rotation = rotate_and_mirror ? -1 : 0;
						// add layers if there are any
						addMapTile(level->background_static_, curTileRect, chosen_texture_set.bg, rotation, rotate_and_mirror);
						addMapTile(level->foreground_static_, curTileRect, chosen_texture_set.fg, rotation, rotate_and_mirror);
						addMapTile(level->foreground_dynamic_, curTileRect, chosen_texture_set.fg_dyn, rotation, rotate_and_mirror);
						//...

						addCollision(curTileRect, chosen_texture_set.collision, texture_atlas_->getTexture(), rotation, rotate_and_mirror);
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
		const int tilesize = road_texture_width_ * background_size_factor;

		sf::IntRect r4 = texture_atlas_->getArea("road-4way");
		// road 3way, not connected dir defines the name, default from texture: 'e'
		sf::IntRect r3_e = texture_atlas_->getArea("road-3way");
		sf::IntRect r3_w(r3_e.left+r3_e.width, r3_e.top+ r3_e.height, -r3_e.width, -r3_e.height);		// mirror both
		// road 2way curve, connected dirs define the name, default from texture: 'se'
		sf::IntRect r2_curve = texture_atlas_->getArea("road-2way");		
		// straight road, connected dirs define the name, default from texture: 'ns'
		sf::IntRect r2_straight = texture_atlas_->getArea("road");

		// crossing connection table for waypoints and stop info within a crossing/node
		// they could be saved within the graph as node_type but since they are always the same anyway, we just save one full crossing info
		initConnectionTable(level->road_network_, tilesize);


		
		auto count = size.x * size.y;
		for (int i = 0; i < count; i++)
		{
			int y = i / size.y;
			int x = i - i * y;
			sf::Color col = map.getPixel(x, y);

			// hit a road
			if (col.toInteger() == tile_colors::road)
			{
				std::vector<std::pair<int, direction>> availableEdgeStarts;
				std::set<int> setVisitedNodes;

				// count road neighbours
				std::vector<sf::Vector2u> road_neighbors;
				std::vector<sf::Vector2u> other_neighbors;
				int neighbor_samecolor_count = gatherPixelNeighborInfo_sameColor(map, x, y, &road_neighbors, &other_neighbors, false);
				
				if (neighbor_samecolor_count > 2 || neighbor_samecolor_count == 1)
				{
					//availableEdgeStarts.push_back(std::make_pair(i, NORTH));
					//availableEdgeStarts.push_back(std::make_pair(i, EAST));
					//availableEdgeStarts.push_back(std::make_pair(i, SOUTH));
					//availableEdgeStarts.push_back(std::make_pair(i, WEST));
					
					for (auto& n : road_neighbors)
					{
						direction dir = NORTH;
						if (n.x == x)
						{
							if (n.y > y)
								dir = SOUTH;
						}
						else
						{
							if (n.x > x)
								dir = EAST;
							else
								dir = WEST;
						}
						
						availableEdgeStarts.push_back(std::make_pair(i, dir));
					}
					setVisitedNodes.insert(i);
				}
				else if(neighbor_samecolor_count == 2)
				{
					// special case, hit straight road first
					// find next node to add to available crossingSlots
				}
				else
				{
					// todo: special case neighbor_count == 0, single road tile
					// nothing to add to available crossingSlots
				}

				while (availableEdgeStarts.size())
				{
					auto pair = availableEdgeStarts.begin();
					availableEdgeStarts.erase(pair);
					int cur_edge_start_id = pair->first;
					std::vector<direction> cur_edge_directions;
					cur_edge_directions.push_back(pair->second);
					int cur_edge_end_id = findNextRoadNode(cur_edge_start_id, cur_edge_directions);

					// gather waypoints and distances
					// addEdge(start, end, waypoints1, dist1)
					// addEdge(end, start, waypoints2, dist2)

					auto end_incoming_dir = cur_edge_directions.back();
					direction before_end_outgoing_dir = (direction)(end_incoming_dir + 2 % direction::DIR_COUNT);

					neighbor_samecolor_count = gatherPixelNeighborInfo_sameColor(map, x, y, &road_neighbors, &other_neighbors, false);
					
					// not visited the found node before
					if (setVisitedNodes.count(i) == 0)
					{
						setVisitedNodes.insert(i);

						// add mew available edges, expect the direction we just used
						for (auto& n : road_neighbors)
						{
							direction dir = NORTH;
							if (n.x == x)
							{
								if (n.y > y)
									dir = SOUTH;
							}
							else
							{
								if (n.x > x)
									dir = EAST;
								else
									dir = WEST;
							}

							if (dir != before_end_outgoing_dir)
								availableEdgeStarts.push_back(std::make_pair(i, dir));
						}						
					}
					// visited the node before -> do not add the edges from that node because they are in already
					else
					{
						// remove the edge that may come from the other side
						std::remove(availableEdgeStarts.begin(), availableEdgeStarts.end(), std::make_pair(cur_edge_end_id, before_end_outgoing_dir));
					}					
				}
			}
		}

		int i = 0;
		for (int x = 0; x < size.x; x++)
		{
			for (int y = 0; y < size.y; y++)
			{
				sf::Color col = map.getPixel(x, y);
				// hit a road
				if (col.toInteger() == tile_colors::road)
				{
					sf::IntRect curTileRect(x*tilesize, y*tilesize, tilesize, tilesize);
					// count road neighbours
					std::vector<sf::Vector2u> road_neighbors;
					std::vector<sf::Vector2u> other_neighbors;
					int neighbor_samecolor_count = gatherPixelNeighborInfo_sameColor(map, x, y, &road_neighbors, &other_neighbors, false);
					
					if (neighbor_samecolor_count == 4)
					{
						addMapTile(level->background_static_, curTileRect, r4);
					}
					else if (neighbor_samecolor_count == 3)
					{
						if (other_neighbors.size() == 1)
						{
							if (other_neighbors.front().x == x + 1)
								addMapTile(level->background_static_, curTileRect, r3_e);
							else if (other_neighbors.front().x == x - 1)
								addMapTile(level->background_static_, curTileRect, r3_w);
							else if (other_neighbors.front().y == y + 1)
								addMapTile(level->background_static_, curTileRect, r3_e, -1);
							else if (other_neighbors.front().y == y - 1)
								addMapTile(level->background_static_, curTileRect, r3_e, 1);
						}
					}
					else
					{
						if (neighbor_samecolor_count == 1)
						{
							auto& pos = road_neighbors.front();
							if (pos.x != x)
								addMapTile(level->background_static_, curTileRect, r2_straight, 1);
							else
								addMapTile(level->background_static_, curTileRect, r2_straight);
						}
						else if (neighbor_samecolor_count == 2)
						{
							// straight
							if (road_neighbors[0].x == road_neighbors[1].x || road_neighbors[0].y == road_neighbors[1].y)
								if( road_neighbors[0].x != x )
									addMapTile(level->background_static_, curTileRect, r2_straight, 1);
								else
									addMapTile(level->background_static_, curTileRect, r2_straight);
							// curve
							else
							{
								auto rect = r2_curve;
								bool v_mirror = road_neighbors[0].x < x || road_neighbors[1].x < x;
								bool h_mirror = road_neighbors[0].y < y || road_neighbors[1].y < y;
								if (v_mirror)
								{
									rect.left += rect.width;
									rect.width *= -1.0f;
								}
								if (h_mirror)
								{
									rect.top += rect.height;
									rect.height *= -1.0f;
								}
								addMapTile(level->background_static_, curTileRect, rect);
							}
						}

						
					}
				}
			}
		}
	}

	int TLevel_Builder::findNextRoadNode(int start_index, std::vector<direction>& edge_directions, sf::Image& map)
	{
		int end_index = start_index;		
		int neighbor_count = 0;
			
		auto size = map.getSize();
		sf::Vector2u cur_pix;
		sf::Vector2u prev_pix;
		prev_pix.y = start_index / size.y;
		prev_pix.x = start_index - start_index * prev_pix.y;

		std::vector<direction> neighbors;
		neighbors.reserve(4);

		do
		{	
			//should always have zero or one, because initialized or neighbor_count == 2 - 1
			if (neighbors.size() == 1)
			{
				edge_directions.push_back(neighbors[0]);
			}

			direction dir = edge_directions.back();
			direction next_in = (direction)(dir + 2 % direction::DIR_COUNT);
			cur_pix = prev_pix;
			if (dir == NORTH)
			{
				cur_pix.y--;
				end_index -= size.y;
			}
			else if (dir == SOUTH)
			{
				cur_pix.y++;
				end_index += size.y;
			}
			else if (dir == EAST)
			{
				cur_pix.x++;
				end_index++;
			}
			else if (dir == WEST)
			{
				cur_pix.x--;
				end_index--;
			}


			

			neighbors.clear();
			neighbor_count = gatherPixelNeighborDirs_sameColor(map, cur_pix.x, cur_pix.y, &neighbors);
			
			edge_directions.push_back(next_in);
			std::remove(neighbors.begin(), neighbors.end(), next_in);
						
			prev_pix = cur_pix;
		} while (neighbor_count == 2 && end_index != start_index);

		return end_index;
	}

	void TLevel_Builder::initConnectionTable(road_network& network, float tilesize)
	{
		auto& connection_table = network.crossing_connection_table;
		//************************************
		// create connection table
		// 0, 1, 2, 3
		// N, E, S, W
		//road_connection_info connection_table[direction::DIR_COUNT][direction::DIR_COUNT];

		// dist = perimeter of the circle / 4 = (pi * 2 * r) / 4 = (pi * 2 * 1/3) / 4 = pi * 2 / 12
		const float dist_short_curve = tilesize * M_PI * 2.0f / 12.0f;
		const float dist_long_curve = dist_short_curve * 2.0f; // tilesize * M_PI * 2.0f * 2.0f / 12.0f;
		//const float dist_straight = tilesize;

		// N>S
		connection_table[NORTH][SOUTH].waypoints.emplace_back(tilesize / 3.0f, 0.0f);
		connection_table[NORTH][SOUTH].waypoints.emplace_back(tilesize / 3.0f, tilesize);
		connection_table[NORTH][SOUTH].distance = tilesize;
		// N>E
		//	circle center (64,0) - radius 1/3
		//	10 steps for waypoint generation 180-270°
		float radius = 2.0f / 3.0f * tilesize;
		float angle = 180.0f * DEG_TO_RAD;
		const float step = 10.0f * DEG_TO_RAD;
		const c2v center{ 64.0f, 0.0f };
		connection_table[NORTH][EAST].waypoints.emplace_back(tilesize - radius, 0.0f);
		for (int i = 1; i < 9; i++)
		{
			angle -= step;
			c2v pt = tgf::math::MathHelper2D::calc_point_on_circle(radius, angle, center);
			connection_table[NORTH][EAST].waypoints.emplace_back(pt.x, pt.y);
		}
		connection_table[NORTH][EAST].waypoints.emplace_back(64.0f, radius);
		connection_table[NORTH][EAST].distance = dist_long_curve;
		// N>W
		//	circle center 0,0 - radius 1/3
		//	10 steps for waypoint generation 0-90°		
		radius = 1.0f / 3.0f * tilesize;
		angle = 0.0f;
		connection_table[NORTH][WEST].waypoints.emplace_back(radius, 0.0f);
		for (int i = 1; i < 9; i++)
		{
			angle += step;
			c2v pt = tgf::math::MathHelper2D::calc_point_on_circle(radius, angle);
			connection_table[NORTH][WEST].waypoints.emplace_back(pt.x, pt.y);
		}
		connection_table[NORTH][WEST].waypoints.emplace_back(0.0f, radius);
		connection_table[NORTH][WEST].distance = dist_short_curve;

		// create rest of table entries by rotating the N > X variant counter-clock wise
		// reserve space for copiing N>W to W>S to S>E to E>N		// 0 N, 1 E, 2 S, 3 W
		connection_table[WEST][SOUTH].waypoints.resize(connection_table[NORTH][WEST].waypoints.size());
		connection_table[SOUTH][EAST].waypoints.resize(connection_table[NORTH][WEST].waypoints.size());
		connection_table[EAST][NORTH].waypoints.resize(connection_table[NORTH][WEST].waypoints.size());
		// reserve space for copiing N>S to W>E to S>N to E>W		// 0 N, 1 E, 2 S, 3 W
		connection_table[WEST][EAST].waypoints.resize(connection_table[NORTH][SOUTH].waypoints.size());
		connection_table[SOUTH][NORTH].waypoints.resize(connection_table[NORTH][SOUTH].waypoints.size());
		connection_table[EAST][WEST].waypoints.resize(connection_table[NORTH][SOUTH].waypoints.size());
		// reserve space for copiing N>E to W>N to S>W to E>S		// 0 N, 1 E, 2 S, 3 W
		connection_table[WEST][NORTH].waypoints.resize(connection_table[NORTH][EAST].waypoints.size());
		connection_table[SOUTH][WEST].waypoints.resize(connection_table[NORTH][EAST].waypoints.size());
		connection_table[EAST][SOUTH].waypoints.resize(connection_table[NORTH][EAST].waypoints.size());

		// copy and rotate 3 connections from N (N>S, N>W, N>E) 3 times each, counter clock wise  P(x,y) -> P'(y, tilesize - x)
		for (int connections = SOUTH; connections < direction::DIR_COUNT; connections++)
		{
			std::vector<sf::Vector2f>& origin = connection_table[NORTH][connections].waypoints;
			int to = connections;
			// starting point from 3 to connection-1, run 3 times always doing -1 on from,to
			for (int from = WEST; from > NORTH; from--)
			{
				to--;
				if (to < 0)
					to = direction::DIR_COUNT - 1;

				// rotate origin counterclock wise (-90°)
				for (size_t i = 0; i < origin.size(); i++)
				{
					connection_table[from][to].waypoints[i].x = origin[i].y;
					connection_table[from][to].waypoints[i].y = tilesize - origin[i].x;
				}

				// reset the origin to the currently filled to further rotate with next interation
				origin = connection_table[from][to].waypoints;
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

	int TLevel_Builder::gatherPixelNeighborDirs_sameColor(const sf::Image & map, const int x, const int y, std::vector<direction>* same_neighbours, std::vector<direction>* other_neighbours)
	{
		auto size = map.getSize();
		if (x < 0 || y < 0 || x >= size.x || y >= size.y)
			return -1;

		int samecolor_neighbor_count = 0;
		const sf::Uint32 col = map.getPixel(x, y).toInteger();

		// check EAST
		if (x + 1 < size.x && map.getPixel(x + 1, y).toInteger() == col)
		{
			samecolor_neighbor_count++;
			if (same_neighbours)
				same_neighbours->push_back(EAST);
		}
		else if (other_neighbours)
			other_neighbours->push_back(EAST);

		// check WEST
		if (x - 1 >= 0 && map.getPixel(x - 1, y).toInteger() == col)
		{
			samecolor_neighbor_count++;
			if (same_neighbours)
				same_neighbours->push_back(WEST);
		}
		else if (other_neighbours)
			other_neighbours->push_back(WEST);

		// check NORTH
		if (y - 1 >= 0 && map.getPixel(x, y - 1).toInteger() == col)
		{
			samecolor_neighbor_count++;
			if (same_neighbours)
				same_neighbours->push_back(NORTH);
		}
		else if (other_neighbours)
			other_neighbours->push_back(NORTH);

		// check SOUTH
		if (y + 1 < size.y && map.getPixel(x, y + 1).toInteger() == col)
		{
			samecolor_neighbor_count++;
			if (same_neighbours)
				same_neighbours->push_back(SOUTH);
		}
		else if (other_neighbours)
			other_neighbours->push_back(SOUTH);

		return samecolor_neighbor_count;
	}

	// vertex arrays are supposed to use PrimitiveType::Quads, rectangular_rotation is multiplied by 90°
	void TLevel_Builder::addMapTile(sf::VertexArray & vertices, sf::IntRect tile_rect, sf::IntRect texture_rect, int rectangular_rotation, bool mirror_horizontally, bool mirror_vertically)
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
		
		/*
		// special case because it is often used in the game:
		// rotate and mirror to keep the correct pixel art lighting 
		if(rectangular_rotation == -1 && mirror_horizontally)
		{
			// rotate by -90°(270°) and mirror horizontally
			// 0 1			-->	3 0			--> 0 3
			// 3 2				2 1				1 2
			vertices[startindex + 3].texCoords = vertices[startindex + 1].texCoords;
			vertices[startindex + 1].texCoords = { (float)texture_rect.left, (float)texture_rect.top + texture_rect.height };
		}
		*/

		int rot = rectangular_rotation % 4;
		if (rot < 0)
			rot += 4;
		
		bool mirror_diagonally = false;
		if (rot == 1)
		{
			// 90 degree rotation can be achieved by mirroring diagonally and h or v for clockwise or counterclockwise
			// see: https://stackoverflow.com/questions/29336374/90-degree-rotation-using-only-reflections
			mirror_vertically = !mirror_vertically;
			mirror_diagonally = true;			
		}
		else if (rot == 3)
		{
			mirror_horizontally = !mirror_horizontally;
			mirror_diagonally = true;
		}
		else if (rot == 2)
		{
			mirror_horizontally = !mirror_horizontally;
			mirror_vertically = !mirror_vertically;
		}

		if (mirror_diagonally)
		{
			// 0 1	--> 0 3
			// 3 2	-->	1 2
			auto temp = vertices[startindex + 3].texCoords;
			vertices[startindex + 3].texCoords = vertices[startindex + 1].texCoords;
			vertices[startindex + 1].texCoords = temp;
		}
		
		if (mirror_horizontally)
		{
			// 0 1	-->	1 0		
			// 3 2	--> 2 3
			auto temp = vertices[startindex + 0].texCoords;
			vertices[startindex + 0].texCoords = vertices[startindex + 1].texCoords;
			vertices[startindex + 1].texCoords = temp;
			temp = vertices[startindex + 2].texCoords;
			vertices[startindex + 2].texCoords = vertices[startindex + 3].texCoords;
			vertices[startindex + 3].texCoords = temp;
		}
		
		if(mirror_vertically)
		{
			// 0 1	-->	3 2		
			// 3 2	--> 0 1
			auto temp = vertices[startindex + 0].texCoords;
			vertices[startindex + 0].texCoords = vertices[startindex + 3].texCoords;
			vertices[startindex + 3].texCoords = temp;
			temp = vertices[startindex + 1].texCoords;
			vertices[startindex + 1].texCoords = vertices[startindex + 2].texCoords;
			vertices[startindex + 2].texCoords = temp;			
		}
	}

	void TLevel_Builder::addCollision(sf::IntRect tile_rect, sf::IntRect collision_texture_data, sf::Texture * tex, int rectangular_rotation, bool mirror_horizontally, bool mirror_vertically)
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