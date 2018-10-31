#include "GameState_Running.h"
#include "GameState_Pause.h"
#include "GameState_End.h"
#include <algorithm>

#include "TLevel.h"
#include "TLevel_Builder.h"
#include "TPlayer.h"
#include "Game.h"
#include "tinyc2.h"

namespace tinytrain
{
	GameState_Running::GameState_Running(tgf::Game * game)
	{
		game_ = game;

		collisionMananger_ = std::make_unique<TTrainCollisionManager>();

		

		player_ = std::make_unique<TPlayer>(this);
		player_->setColor(sf::Color::Green);

		camera_ = std::make_unique<sf::View>();

		// CAMERA SETTINGS
		bRotateCameraWithTrack_ = false;
		camFlowTime_ = 1.0;
		camCurrentTime_ = 0.0;

		// todo: level selection

		// LEVEL 1 - works correct
		TLevel::level_info info_level1;
		//info_level1.header = "Level 1"
		info_level1.introduction_text = "Find your way through the traffic to bring your passengers to their destination station!";
		info_level1.map_file = "data/images/level/map.png";
		info_level1.car_count = 10;
		info_level1.passenger_count = 0;
		info_level1.inital_wagon_count = 3;
		info_level1.start_pts.emplace_back(sf::FloatRect(13.0f, 2.0f, 1.0f, 1.0f), direction::SOUTH, sf::IntRect(0.0f, 0.0f, 0.0f, 0.0f));
		info_level1.stations.emplace_back(sf::FloatRect(7.0f,17.0f,1.0f,1.0f), sf::IntRect(0.0f, 0.0f, 0.0f, 0.0f));
				
		info_level1.deco_images.emplace_back(sf::Vector2f(15.0f, 2.0f), "info_mouse1");
		info_level1.deco_images.emplace_back(sf::Vector2f(13.0f, 0.0f), "info_steering");
		info_level1.deco_images.emplace_back(sf::Vector2f(10.0f, 2.0f), "info_pause");

		info_level1.points_to_reach = 0;
		info_level1.timelimit = 0.0f;

		// LEVEL 2 - works correct
		TLevel::level_info info_level2;
		//info_level2.header = "Level 2"
		info_level2.introduction_text = "Find your way through the traffic to your destination station!";
		info_level2.map_file = "data/images/level/map.png";
		info_level2.car_count = 10;
		info_level2.passenger_count = 0;
		info_level2.inital_wagon_count = 3;
		info_level2.start_pts.emplace_back(sf::FloatRect(13.0f, 2.0f, 1.0f, 1.0f), direction::SOUTH, sf::IntRect(0.0f, 0.0f, 0.0f, 0.0f));
		info_level2.stations.emplace_back(sf::FloatRect(7.0f, 17.0f, 1.0f, 1.0f), sf::IntRect(0.0f, 0.0f, 0.0f, 0.0f));
		info_level2.points_to_reach = 0;
		info_level2.timelimit = 40.0f;

		// LEVEL 3 - DOESNT WORK passengers missing
		TLevel::level_info info_level3;
		//info_level3.header = "Level 3"
		info_level3.introduction_text = "Find your way through the traffic to bring at least 3 passengers to their destinations before going home!";
		info_level3.map_file = "data/images/level/map.png";
		info_level3.car_count = 10;
		info_level3.passenger_count = 5;
		info_level3.inital_wagon_count = 3;
		info_level3.start_pts.emplace_back(sf::FloatRect(13.0f, 2.0f, 1.0f, 1.0f), direction::SOUTH, sf::IntRect(0.0f, 0.0f, 0.0f, 0.0f));
		info_level3.stations.emplace_back(sf::FloatRect(7.0f, 17.0f, 1.0f, 1.0f), sf::IntRect(0.0f, 0.0f, 0.0f, 0.0f));
		info_level3.points_to_reach = 3;
		info_level3.timelimit = 150.0f;

		// LEVEL 4 - DOESNT WORK passengers, wagon "pickups" and other objectives missing
		TLevel::level_info info_level4;
		//info_level4.header = "Level 4"
		info_level4.introduction_text = "Find your way through the traffic to bring your passengers to their destination station!";
		info_level4.map_file = "data/images/level/map.png";
		info_level4.car_count = 10;
		info_level4.passenger_count = 10;
		info_level4.inital_wagon_count = 3;
		info_level4.start_pts.emplace_back(sf::FloatRect(13.0f, 2.0f, 1.0f, 1.0f), direction::SOUTH, sf::IntRect(0.0f, 0.0f, 0.0f, 0.0f));
		info_level4.stations.emplace_back(sf::FloatRect(7.0f, 17.0f, 1.0f, 1.0f), sf::IntRect(0.0f, 0.0f, 0.0f, 0.0f));
		info_level4.points_to_reach = 50;
		info_level4.timelimit = 0.0f;

		loadLevel(info_level3);
		//loadLevel(info_level1);
		
		gui_ = std::make_unique<gui::TLevelInfo_HUD>(level_.get(), *(game->font_));
		if (game && game->window_)
		{
			auto size = game->window_->getSize();
			gui_->recalcHUDPositions(size.x, size.y);
		}		
	}

	GameState_Running::~GameState_Running()
	{
		if (level_)
		{
			auto rail = level_->railtrack_.get();
			if (rail)
				rail->unbindAllCallbacks(this);
		}
	}

	void GameState_Running::update(float deltaTime)
	{
		if (level_)
		{
			// update level
			level_->update(deltaTime);

			// update collisions
			collisionMananger_->update();

			// update camera
			if (camera_)
			{
				camCurrentTime_ += deltaTime;
				float u = 1.0f;
				if (camCurrentTime_ < camFlowTime_)
					u = camCurrentTime_ / camFlowTime_;

				// move/rotate camera when needed
				if (camNewPos_ != camOldPos_)
				{
					c2v start{ camOldPos_.x, camOldPos_.y };
					c2v end	 { camNewPos_.x, camNewPos_.y };

					start = c2Lerp(start, end, u);

					camera_->setCenter(start.x, start.y);
				}
				if (camNewRot_ != camOldRot_)
				{
					camera_->setRotation(camOldRot_ + (camNewRot_ - camOldRot_)*u);
				}
			}			

			// update player (mouse input to spline, hud info)
			if (player_)
				player_->update(deltaTime);

			if (gui_)
				gui_->update(deltaTime);
		}
	}

	void GameState_Running::draw(sf::RenderTarget * target)
	{
		if (target == nullptr)
			return;

		// gameview
		target->setView(*camera_);

		// draw level
		if (level_)
			level_->draw(target);
		
		// guiview
		if (game_)
			target->setView(*game_->guiView_);

		// draw gui
		if (gui_)
			gui_->draw(target);
		
		// draw player (drawing rect)
		if(player_)
			player_->draw(target);

		// destroying the player on purpose (for testing purposes)
		//player_.reset(nullptr);
	}
	
	void GameState_Running::onWindowSizeChanged(int w, int h)
	{
		if (camera_)
			camera_->setSize(w, h);

		player_->onWindowSizeChanged(w, h);

		gui_->recalcHUDPositions(w, h);
	}

	void GameState_Running::initCurrentLevel()
	{
		if (game_ && camera_ && game_->window_ && player_)
		{
			// initial camera view matching the window in size and position (coordinate system match)
			sf::Vector2f size = sf::Vector2f(game_->window_->getSize());
			camera_->setSize(size);
			camera_->setCenter(size*0.5f);

			// init drawrect and set railtrack for the player
			player_->onWindowSizeChanged(size.x, size.y);
			if (level_)
			{
				auto rail = level_->railtrack_.get();
				player_->setTrack(rail);

				if (rail)
					rail->bindTrackChangedCallback(this, &GameState_Running::moveCameraToLastRail);
			}

			// initially move camera to end of rail
			moveCameraToLastRail();
		}
		else
			printf("GAMESTATE INITIALIZING ERROR.\n");
	}

	// this can be a used by a callback when the rail has been changed
	void GameState_Running::moveCameraToLastRail()
	{
		auto track = level_->railtrack_->getTrackSpline();
		if (track)
		{
			int hintindex = -1;
			float angle = 0.0f;
			if (bRotateCameraWithTrack_)
			{
				angle = track->getDirectionAngleAtTime(1.0, hintindex, false);
				// SFML Coordinate System correction
				angle += 90.0f;
			}

			auto pos = track->getLocationAtTime(1.0, hintindex);
			moveCameraToPoint(pos, angle, camFlowTime_);
		}
	}

	void GameState_Running::moveCameraToPoint(sf::Vector2f pos, float angle, float time)
	{
		camCurrentTime_ = 0.0f;
		camFlowTime_ = time;
		camNewPos_ = pos;
		camOldPos_ = camera_->getCenter();
		camNewRot_ = angle;
		camOldRot_ = camera_->getRotation();

		// clamp rotation
		float totalrot = camOldRot_ - camNewRot_;
		if (totalrot > 180.0f)
			camOldRot_ -= 360.0f;
		else if (totalrot < -180.0f)
			camOldRot_ += 360.0f;
	}


	void GameState_Running::won(TTrain * train)
	{
		
#if 0		// 0 for ghost mode
		train->pause();

		if (game_)
		{
			auto endscreen = std::make_unique<GameState_End>(game_);
			endscreen->backgroundstates_.push_back(this);
			endscreen->setEndText("you won!", sf::Color::Green);

			game_->pushState(std::move(endscreen));
		}
#endif
		printf("you win.\n");
	}
	void GameState_Running::lost(TTrain * train)
	{
#if 0		// 0 for ghost mode
		train->pause();
		if (game_)
		{
			auto endscreen = std::make_unique<GameState_End>(game_);
			endscreen->backgroundstates_.push_back(this);
			endscreen->setEndText("you lost!", sf::Color::Red);

			game_->pushState(std::move(endscreen));
		}
#endif
		printf("you loose.\n");
	}

	void GameState_Running::pause()
	{
		if (game_)
		{
			auto pause = std::make_unique<GameState_Pause>(game_);
			pause->backgroundstates_.push_back(this);

			game_->pushState(std::move(pause));

			printf("paused.\n");
		}
	}

	void GameState_Running::restart()
	{
		//if(level_)
		//	level_->restart();
		
		//loadLevel();

		//initCurrentLevel();
	}

	void GameState_Running::decreaseTime(float seconds)
	{
		if (level_ && level_->elapsed_time_)
		{
			level_->elapsed_time_ += seconds;
			if (level_->elapsed_time_ < 0.0f)
				level_->elapsed_time_ = 0.0f;
		}
	}

	void GameState_Running::loadLevel(const TLevel::level_info & info)
	{
		TLevel_Builder level_gen(this);

		level_ = level_gen.loadLevel(info);
		if(level_)
			initCurrentLevel();
	}

	TTrainCollisionManager* GameState_Running::getCollisionManager()
	{
		return collisionMananger_.get();
	}
}