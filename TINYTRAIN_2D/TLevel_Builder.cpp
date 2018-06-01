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

		int t1 = std::clock();

		// every pixel is an area of the size of a (simple) street
		const auto size = map.getSize();
		const int tilesize = road_texture_width_ * background_size_factor;

		sf::IntRect r4 = texture_atlas_->getArea("road-4way");
		// road 3way, not connected dir_prevTile_out defines the name, default from texture: 'e'
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
			auto p = tgf::math::MathHelper2D::getArrayCoordsFromIndex(i, size.x);
			int y = p.second;
			int x = p.first;
			sf::Color col = map.getPixel(x, y);

			// hit a road
			if (col.toInteger() == tile_colors::road)
			{
				std::vector<std::pair<int, direction>> availableEdgeStarts;
				std::vector<std::tuple<int, int, std::vector<direction>>> foundEdges;
				std::set<int> setVisitedNodes;

				// count road neighbours
				std::vector<direction> road_neighbors;
				//std::vector<sf::Vector2u> other_neighbors;
				int neighbor_samecolor_count = gatherPixelNeighborDirs_sameColor(map, x, y, &road_neighbors);//, &other_neighbors, false);
				
				
				if (neighbor_samecolor_count > 2 || neighbor_samecolor_count == 1)
				{
					for (auto& n : road_neighbors)
						availableEdgeStarts.push_back(std::make_pair(i, n));

					setVisitedNodes.insert(i);
				}				
				else if (neighbor_samecolor_count == 2)
				{
					// special case, hit straight road first
					// find next node to add to available crossingSlots
					std::vector<direction> dirs;
					dirs.push_back(road_neighbors.front());

					int startnode = findNextRoadNode(i, dirs, map);

					if (startnode == i)
					{
						printf("road generation warning: road loop detected. may not be intentional?\n");
						// loop only, no other crossings or deadends involved
						setVisitedNodes.insert(startnode);
						auto dir = dirs.rbegin();
						if (dir != dirs.rend())
							availableEdgeStarts.push_back(std::make_pair(startnode, *dir));
					}
					else
					{
						// redo neighbor calc, so that the crossing/deadend functionality can be initiated
						p = tgf::math::MathHelper2D::getArrayCoordsFromIndex(startnode, size.x);
						neighbor_samecolor_count = gatherPixelNeighborDirs_sameColor(map, p.first, p.second, &road_neighbors);//, &other_neighbors, false);

						for (auto& n : road_neighbors)
							availableEdgeStarts.push_back(std::make_pair(startnode, n));

						setVisitedNodes.insert(startnode);
					}
				}
				else
				{
					printf("road generation warning: single road tile detected. may not be intentional?\n");
					// special case neighbor_count == 0, single road tile
					// just add a node 
					level->road_network_.road_graph.addVertex(i);
					setVisitedNodes.insert(i);

					//// maybe add an edge with no waypoints from SOUTH to NORTH, so a car could actually loop?
					//edge_info e;
					//e.in_slot = NORTH;
					//e.out_slot = SOUTH;
					//level->road_network_.road_graph.addEdge(i, i, 0.0f, e);
				}

				// gather edges
				while (availableEdgeStarts.size())
				{
					auto pair = availableEdgeStarts.begin();
					
					int cur_edge_start_id = pair->first;
					std::vector<direction> cur_edge_directions;
					cur_edge_directions.push_back(pair->second);

					availableEdgeStarts.erase(pair);
					
					int cur_edge_end_id = findNextRoadNode(cur_edge_start_id, cur_edge_directions, map);

					// save edge start/end/dirs to later do:
					foundEdges.emplace_back(cur_edge_start_id, cur_edge_end_id, cur_edge_directions);
					auto end_incoming_dir = cur_edge_directions.back();
					

					road_neighbors.clear();
					p = tgf::math::MathHelper2D::getArrayCoordsFromIndex(cur_edge_end_id, size.x);
					y = p.second;
					x = p.first;
					neighbor_samecolor_count = gatherPixelNeighborDirs_sameColor(map, x, y, &road_neighbors);
					
					// not visited the found node before
					if (setVisitedNodes.count(cur_edge_end_id) == 0)
					{
						setVisitedNodes.insert(cur_edge_end_id);

						// add new available edges, except the direction we just came from
						for (auto& n : road_neighbors)
						{
							if (n != end_incoming_dir)
								availableEdgeStarts.push_back(std::make_pair(cur_edge_end_id, n));
						}						
					}
					// visited the node before -> do not add the edges from that node because they are in already
					else
					{
						// remove the available edge that may come from the end side into the direction we came in
						availableEdgeStarts.erase(std::remove(availableEdgeStarts.begin(), availableEdgeStarts.end(), std::make_pair(cur_edge_end_id, end_incoming_dir)), availableEdgeStarts.end() );
					}					
				}
			
				// process crossing tiles, unset pixel colors for those
				for (auto& idx : setVisitedNodes)
				{
					auto p = tgf::math::MathHelper2D::getArrayCoordsFromIndex(idx, size.x);
					sf::Vector2u coords(p.first, p.second);
					
					sf::IntRect curTileRect(coords.x*tilesize, coords.y*tilesize, tilesize, tilesize);

					std::vector<direction> same;
					std::vector<direction> others;
					int count = gatherPixelNeighborDirs_sameColor(map, coords.x, coords.y, &same, &others);
					if (count == 4)
						addMapTile(level->background_static_, curTileRect, r4);
					else if (count == 3)
					{
						if (others.size() == 1)
						{
							if (others.front() == EAST)
								addMapTile(level->background_static_, curTileRect, r3_e);
							else if (others.front() == WEST)
								addMapTile(level->background_static_, curTileRect, r3_w);
							else if (others.front() == SOUTH)
								addMapTile(level->background_static_, curTileRect, r3_e, -1);
							else if (others.front() == NORTH)
								addMapTile(level->background_static_, curTileRect, r3_e, 1);
						}
					}
					else if (count == 2)
					{
						// special case, this can actually only happen when a single loop with no crossings is in the image
						direction dir1 = same[0];
						direction dir2 = same[1];

						// sum is even -> straight (0+2, 1+3)
						if((dir1+dir2)%2 == 0)
						{
							// straight
							if (dir1 == NORTH || dir1 == SOUTH)
								addMapTile(level->background_static_, curTileRect, r2_straight);
							else
								addMapTile(level->background_static_, curTileRect, r2_straight, 1);
						}
						// sum is odd -> curve (0+1, 0+3, 1+2, 2+3)
						else
						{
							// curve
							bool mirror_v = false;
							bool mirror_h = false;

							if (dir1 == NORTH || dir2 == NORTH)
								mirror_v = true;
							if (dir1 == WEST || dir2 == WEST)
								mirror_h = true;

							addMapTile(level->background_static_, curTileRect, r2_curve, 0, mirror_h, mirror_v);
						}
					}
					else if (count == 1)
					{
						if (same.size() == 1)
						{
							// deadend
							if (same.front() == EAST || same.front() == WEST)
								addMapTile(level->background_static_, curTileRect, r2_straight, 1);
							else //if (same.front() == NORTH || same.front() == SOUTH)
								addMapTile(level->background_static_, curTileRect, r2_straight);
						}
					}
					// may be single road tiles
					else
						addMapTile(level->background_static_, curTileRect, r2_straight);
				}
				for (auto& idx : setVisitedNodes)
				{
					auto p = tgf::math::MathHelper2D::getArrayCoordsFromIndex(idx, size.x);
					sf::Vector2u coords(p.first, p.second);
					// unset road pixel color
					map.setPixel(coords.x, coords.y, sf::Color::Transparent);
				}					
				
				// process edges (add graph info, add map tile, unset pixel color)
				for (auto& e : foundEdges)
				{
					int start = std::get<0>(e);
					int end = std::get<1>(e);
					std::vector<direction> dirs = std::get<2>(e);

					auto p = tgf::math::MathHelper2D::getArrayCoordsFromIndex(start, size.x);
					sf::Vector2u coords(p.first, p.second);

					float dist1 = 0.0f;
					float dist2 = 0.0f;
					edge_info edge_info1, edge_info2;
					
					edge_info1.out_slot = dirs.front();
					edge_info1.in_slot = dirs.back();
					edge_info2.out_slot = edge_info1.in_slot;
					edge_info2.in_slot = edge_info1.out_slot;


					// leave out first and last element in the direction array and do steps = 2
					for(int i = 1; i < dirs.size()-2; i+=2)
					{
						direction dir_prevTile_out = dirs[i - 1];
						direction from = dirs[i];
						direction to = dirs[i + 1];

						// move to next tile
						if (dir_prevTile_out == NORTH) coords.y--;
						else if (dir_prevTile_out == SOUTH) coords.y++;
						else if (dir_prevTile_out == WEST) coords.x--;
						else if (dir_prevTile_out == EAST) coords.x++;

						sf::IntRect curTileRect(coords.x*tilesize, coords.y*tilesize, tilesize, tilesize);

						// gather waypoints (both directions) and distances
						auto& info = level->road_network_.crossing_connection_table[from][to];
						auto& reverse = level->road_network_.crossing_connection_table[to][from];
						for (auto& pt : info.waypoints)
							edge_info1.waypoints.emplace_back(pt.x + curTileRect.left, pt.y + curTileRect.top);
						dist1 += info.distance;

						// filling the waypoint vector from reverse direction from back to front to be able to use emplace_back (fast) 
						// and std::reverse once the edge_info2 is complete
						for (auto pt = reverse.waypoints.rbegin(); pt != reverse.waypoints.rend(); ++pt)
							edge_info2.waypoints.emplace_back(pt->x + curTileRect.left, pt->y + curTileRect.top);
						dist2 += reverse.distance;

						// addMapTiles						
						if (dir_prevTile_out == dirs[i + 1])
						{
							// straight
							if (dir_prevTile_out == NORTH || dir_prevTile_out == SOUTH)
								addMapTile(level->background_static_, curTileRect, r2_straight);
							else
								addMapTile(level->background_static_, curTileRect, r2_straight, 1);
						}
						else
						{
							// curve
							bool mirror_v = false;
							bool mirror_h = false;
							
							if (from == NORTH || to == NORTH)
								mirror_v = true;
							if (from == WEST || to == WEST)
								mirror_h = true;

							addMapTile(level->background_static_, curTileRect, r2_curve, 0, mirror_h, mirror_v);
						}
						// removePixels
						map.setPixel(coords.x, coords.y, sf::Color::Transparent);
					}

					// reverse the edge_info2.waypoints vector because we filled it the wrong way around for performance reasons
					std::reverse(edge_info2.waypoints.begin(), edge_info2.waypoints.end());
					
					// add edges to road_network
					level->road_network_.road_graph.addEdge(start, end, dist1, edge_info1);
					level->road_network_.road_graph.addEdge(end, start, dist2, edge_info2);
				}
			}
		}

		// add road debug stuff
		level->roads_debug_.clear();
		level->roads_debug_.setPrimitiveType(sf::PrimitiveType::Lines);
		const float halftile = tilesize * 0.5f;
		size_t edgecount = 0;
		for (auto& n : level->road_network_.road_graph.nodes_)
		{
			auto p = tgf::math::MathHelper2D::getArrayCoordsFromIndex(n.first, size.x);
			sf::Vector2f pt_start(p.first * tilesize + halftile, p.second * tilesize + halftile);
			for (auto& e : n.second.edges_)
			{
				edgecount++;
				p = tgf::math::MathHelper2D::getArrayCoordsFromIndex(e.target_node_, size.x);
				sf::Vector2f pt_end(p.first * tilesize + halftile, p.second * tilesize + halftile);
				level->roads_debug_.append(sf::Vertex(pt_start, sf::Color::Blue));
				level->roads_debug_.append(sf::Vertex(pt_end, sf::Color::Blue));

				sf::Color col = sf::Color::Magenta;
				for (int i = 1; i < e.user_data_.waypoints.size(); i++)
				{
					sf::Vector2f pt_a = e.user_data_.waypoints[i - 1];
					sf::Vector2f pt_b = e.user_data_.waypoints[i];
					level->roads_debug_.append(sf::Vertex(pt_a, col));
					level->roads_debug_.append(sf::Vertex(pt_b, col));
				}
			}
		}

		int time = std::clock() - t1;
		printf("road generation from %ix%i image took %i ms. %zi nodes and %zi edges placed.\n", size.x, size.y, time, level->road_network_.road_graph.nodes_.size(), edgecount);
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		// benchmark map_big (419x235px)
		//		simple tile generation - 580ms 
		//		tile and roadgraph generation - 2800ms		(5x)
	}

	int TLevel_Builder::findNextRoadNode(int start_index, std::vector<direction>& edge_directions, sf::Image& map)
	{
		int end_index = start_index;		
		int neighbor_count = 0;
			
		auto size = map.getSize();
		sf::Vector2u cur_pix;
		sf::Vector2u ;
		auto p = tgf::math::MathHelper2D::getArrayCoordsFromIndex(start_index, size.x);
		sf::Vector2u prev_pix(p.first, p.second);
		
		std::vector<direction> neighbors;
		neighbors.reserve(4);

		do
		{	
			direction dir = edge_directions.back();
			direction next_in = (direction)((dir + 2) % direction::DIR_COUNT);
			cur_pix = prev_pix;
			if (dir == NORTH)
			{
				cur_pix.y--;
				end_index -= size.x;
			}
			else if (dir == SOUTH)
			{
				cur_pix.y++;
				end_index += size.x;
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

			// when neighbor_count == 2 and remove the next_in before adding the other
			if (neighbors.size() == 2)
			{
				if(neighbors[0] == next_in)
					edge_directions.push_back(neighbors[1]);
				else
					edge_directions.push_back(neighbors[0]);				
			}
				
						
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
		
		// N>E
		//	circle center (64,0) - radius 1/3
		//	10 steps for waypoint generation 180-270�
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
		
		// N>W
		//	circle center 0,0 - radius 1/3
		//	10 steps for waypoint generation 0-90�		
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
		for (int connections = EAST; connections < direction::DIR_COUNT; connections++)
		{
			std::vector<sf::Vector2f>* origin = &connection_table[NORTH][connections].waypoints;
			int to = connections;
			// starting point from 3 to connection-1, run 3 times always doing -1 on from,to
			for (int from = WEST; from > NORTH; from--)
			{
				to--;
				if (to < 0)
					to = direction::DIR_COUNT - 1;

				// rotate origin counterclock wise (-90�)
				for (size_t i = 0; i < origin->size(); i++)
				{
					connection_table[from][to].waypoints[i].x = origin->at(i).y;
					connection_table[from][to].waypoints[i].y = tilesize - origin->at(i).x;
				}

				// reset the origin to the currently filled to further rotate with next interation
				origin = &connection_table[from][to].waypoints;
			}
		}

		// save distances
		connection_table[NORTH][SOUTH].distance = tilesize;
		connection_table[SOUTH][NORTH].distance = tilesize;
		connection_table[EAST][WEST].distance	= tilesize;
		connection_table[WEST][EAST].distance	= tilesize;

		connection_table[NORTH][EAST].distance = dist_long_curve;
		connection_table[EAST][SOUTH].distance = dist_long_curve;
		connection_table[SOUTH][WEST].distance = dist_long_curve;
		connection_table[WEST][NORTH].distance = dist_long_curve;

		connection_table[NORTH][WEST].distance = dist_short_curve;
		connection_table[EAST][NORTH].distance = dist_short_curve;
		connection_table[SOUTH][EAST].distance = dist_short_curve;
		connection_table[WEST][SOUTH].distance = dist_short_curve;


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
		if (same_neighbours)
			same_neighbours->clear();
		if (other_neighbours)
			other_neighbours->clear();

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

	// vertex arrays are supposed to use PrimitiveType::Quads, rectangular_rotation is multiplied by 90�
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
			// rotate by -90�(270�) and mirror horizontally
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
		//		  /			<---x angle (180�) + cross.angle
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