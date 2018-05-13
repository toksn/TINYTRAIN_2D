#include "tinyc2.h"
#include "TLevel.h"
#include "TTrain.h"
#include "TRailTrack.h"
#include "TObstacle.h"
#include "InterpolateToPoint.h"
#include "SplineTexture.h"
#include "GameState_Running.h"

#define city_size_factor 2.0f;

namespace tinytrain
{
	TLevel::TLevel(GameState_Running* gs)
	{
		gs_ = gs;
		
		if (gs_ && gs_->game_)
			texture_atlas_ = gs_->game_->getTextureAtlas();

		drawDebug_ = false;
		if (gs)
		{
			gs->bindEventCallback(sf::Event::KeyPressed, this, &TLevel::onKeyPressed);
			//...
		}
	}


	TLevel::~TLevel()
	{
		if (gs_)
			gs_->unbindAllCallbacks(this);
	}

	void TLevel::onDraw(sf::RenderTarget * target)
	{
		target->draw(roads_, sf::RenderStates::RenderStates(texture_atlas_->getTexture()));
		if(drawDebug_)
			target->draw(roads_debug_);
		

		if (railtrack_)
			railtrack_->draw(target);
		if (train_)
			train_->draw(target);
		
		for (int i = obstacles_.size() - 1; i >= 0; i--)
		{
			auto o = obstacles_[i].get();
			if (o)
				o->draw(target);
			else
				obstacles_.erase(obstacles_.begin() + i);
		}
	}

	void TLevel::onUpdate(float deltaTime)
	{
		if (train_)
			train_->update(deltaTime);
		if (railtrack_)
			railtrack_->update(deltaTime);

		for (int i = obstacles_.size()-1; i >= 0; i--)
		{
			auto o = obstacles_[i].get();
			if (o)
				o->update(deltaTime);
			else
				obstacles_.erase(obstacles_.begin()+i);
		}			
	}

	void TLevel::onKeyPressed(sf::Event& e)
	{
		// PAUSE
		if (e.key.code == sf::Keyboard::F11)
		{
			// toggle debug drawing
			drawDebug_ = !drawDebug_;
		}
	}

	void TLevel::load(std::string file)
	{
		if (gs_)
		{
			if (file.empty())
			{
				generateLevel_random();
			}
			else
			{
				// try to load the file as an image
				sf::Image map;
				if (map.loadFromFile(file))
				{
					generateLevel_fromImage(map);
				}
			}
		}
	}
	
	void TLevel::restart()
	{
		obstacles_.clear();
		load();			//load(currentLevelFile);
	}

	void TLevel::generateLevel_fromImage(sf::Image& map)
	{
		// define possible tile type colors
		// todo: move definition to a fitting place
		//sf::Color tile_types[5];
		const sf::Color water		(  0, 162, 232);	//blue
		const sf::Color residental	(237,  28,  36);	//red
		const sf::Color park		(181, 230,  29);	//bright green
		const sf::Color forest		( 34, 177,  76);	//dark green
		const sf::Color road		(127, 127, 127);	//grey
		const sf::Color industrial	(255, 201,  14);	//yellow

		
		// todo: collect texture rects for every type from atlas (by name)
		std::map < sf::Color, tile_type_info> texture_rects_by_tiletype;
		


		// every pixel is an area of the size of a (simple) street
		auto size = map.getSize();
		int tilesize = 128;
		for (int x = 0; x < size.x; x++)
		{
			for (int y = 0; y < size.y; y++)
			{
				sf::Color col = map.getPixel(x, y);
				sf::IntRect curTileRect(x*tilesize, y*tilesize, tilesize, tilesize);

				//auto it = texture_rects_by_tiletype.find(col);
				//if (it != texture_rects_by_tiletype.end())
				auto cur_type_data = texture_rects_by_tiletype[col];

				if (cur_type_data.isValid)
				{
					// common tile
					addMapTile(background_static, curTileRect, cur_type_data.common_bg, false);


					if (cur_type_data.tex_coords.size())
					{
						// get random element from the list
						int index = rand() % cur_type_data.tex_coords.size();
						tile_type_info::texture_layer_set chosen_texture_set = cur_type_data.tex_coords[index];

						// add layers if there is any
						addMapTile(background_static, curTileRect, chosen_texture_set.bg, cur_type_data.rotationAllowed);
						addMapTile(foreground_static, curTileRect, chosen_texture_set.fg, cur_type_data.rotationAllowed);
						addMapTile(foreground_dynamic, curTileRect, chosen_texture_set.fg_dyn, cur_type_data.rotationAllowed);
						//...

						addCollision(curTileRect, chosen_texture_set.collision, texture_atlas_->getTexture());
					}
				}

				// this should generate:
				//		- background of the tile (texture)
				//		- road_network
				//		- foreground of the tile (something special like a bridge, roofs of houses ect)
				//		- obstacles from collision textures
				//		- possible randomly placed obstacles like trees
				//		- tile type
				//generateLevelTile(col, area);
			}
		}

		// random start location
		// random yellow events (collectables, like passengers, construction_workers, bonus_points)
		// random target zones
	}

	// vertex arrays are supposed to use PrimitiveType::Quads
	void TLevel::addMapTile(sf::VertexArray & vertices, sf::IntRect tile_rect, sf::IntRect texture_rect, bool rotationAllowed)
	{
		int startindex = vertices.getVertexCount();

		// add four points for the vertex array
		vertices.resize(vertices.getVertexCount() + 4);

		vertices[startindex + 0].position = { tile_rect.left, tile_rect.top };
		vertices[startindex + 1].position = { tile_rect.left + tile_rect.width, tile_rect.top };
		vertices[startindex + 2].position = { tile_rect.left + tile_rect.width, tile_rect.top + tile_rect.height };
		vertices[startindex + 3].position = { tile_rect.left, tile_rect.top + tile_rect.height };

		// texure coordinates
		vertices[startindex + 0].texCoords = { texture_rect.left, texture_rect.top };
		vertices[startindex + 1].texCoords = { texture_rect.left + texture_rect.width, texture_rect.top };
		vertices[startindex + 2].texCoords = { texture_rect.left + texture_rect.width, texture_rect.top + texture_rect.height };
		vertices[startindex + 3].texCoords = { texture_rect.left, texture_rect.top + texture_rect.height };
			
		// rotate and mirror to keep the correct pixel art lighting 
		if (rotationAllowed && rand() % 2)
		{
			// rotate by -90�(270�) and mirror horizontally
			// 0 1			-->	3 0			--> 0 3
			// 3 2				2 1				2 1
			vertices[startindex + 3].texCoords = vertices[startindex + 1].texCoords;
			vertices[startindex + 2].texCoords = vertices[startindex + 3].texCoords;
			vertices[startindex + 1].texCoords = { texture_rect.left + texture_rect.width, texture_rect.top + texture_rect.height };
		}
	}

	void TLevel::addCollision(sf::IntRect tile_rect, sf::IntRect collision_texture_data, sf::Texture * tex)
	{
		// circle the texture for black pixels
		// expand pixels until completed area is found

		// use area to create an obstacle (cPoly?)
	}


	void TLevel::generateLevel_random()
	{
		/**************************************************************************
		SIMPLE LEVEL CREATED BY CODE -- this is the minimum requirement for a level
		***************************************************************************/
		tgf::utilities::CityGenerator city;
		tgf::utilities::cgSettings settings;

		settings.road_crossingMinDist *= city_size_factor;
		settings.road_segLength *= city_size_factor;
		settings.road_chanceToSplitRadius *= city_size_factor;
		settings.road_chanceToContinueRadius *= city_size_factor;

		//settings.road_chanceToSplitRadius *= 5.0f;
		//settings.road_chanceToContinueRadius *= 5.0f;
		city.applySettings(settings);

		auto t1 = std::clock();
		city.generate();

		roads_debug_.clear();
		roads_debug_.setPrimitiveType(sf::PrimitiveType::Lines);
		for (auto& road : city.road_segments_)
		{
			roads_debug_.append(sf::Vertex(road->a, road->col_a));
			roads_debug_.append(sf::Vertex(road->b, road->col_b));
		}

		int time = std::clock() - t1;
		printf("road generation took %i ms. %zi segments placed making %fms per segment\n", time, city.road_segments_.size(), (float)time / (float)(city.road_segments_.size()));

		roads_ = triangulateRoadSegments(city);

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
		/*
		c2v seg = c2Sub(end, start);
		// 57.295779513 := rad to degre conversion (rad * 180.0/pi)
		float angle = atan2(seg.y, seg.x) * RAD_TO_DEG;

		for (size_t i = 0; i < 10; i++)
		{
		angle += ((rand() % angle_range) - angle_range * 0.5f)/100.0f;

		//lastPos.x += rand() % 200 - 100;
		//lastPos.y += rand() % 200 - 100;

		//lastPos.x += rand() % 30;
		//lastPos.y += rand() % 30;

		start = end;
		end.x += dist * cos(angle / RAD_TO_DEG);
		end.y += dist * sin(angle / RAD_TO_DEG);

		railtrack_->append(sf::Vector2f(end.x, end.y));
		}
		*/
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



		/************************************************************************/

		// TODO: passengers to pick up
	}

	sf::VertexArray TLevel::triangulateRoadSegments(tgf::utilities::CityGenerator& city)
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
		float streetwidth = 6.4f * city_size_factor;
		//float crossing_radius = streetwidth
		std::vector<sf::VertexArray> tris;


		printf("road triangulation begin: crossings count %zi\n", city.road_crossings_.size());

		//while (city.road_deadends_.size())
		while(city.road_crossings_.size() || city.road_deadends_.size())
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
			else if(city.road_crossings_.size())
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
			if(ctrlPts.empty() && it != city.road_segments_.end())
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
			else if(ctrlPts.size())
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
	bool TLevel::triangulation_insertSplineCtrlPtsForSegmentAtCrossing(tgf::utilities::roadsegment* seg, tgf::utilities::road_crossing* crossing, std::vector<sf::Vector2f>& ctrl_pts, bool start)
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
		float distance = 6.4f * city_size_factor;	// = streetwidth
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
}