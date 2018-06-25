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
		loadLevel("data/images/level/map.png");
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

		player_->recalcDrawRect(w, h);
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
			player_->recalcDrawRect(size.x, size.y);
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
		loadLevel();

		//initCurrentLevel();
	}

	void GameState_Running::loadLevel(std::string file)
	{
		//level_->load("data/images/level/map.png");
		TLevel_Builder level_gen(this);

		if (file.empty())
		{
			level_ = level_gen.generateLevel_random();
		}
		else
		{
			// try to load the file as an image
			sf::Image map;
			if (map.loadFromFile(file))
			{
				level_ = level_gen.generateLevel_fromImage(map);
			}
		}

		if(level_)
			initCurrentLevel();
	}

	TTrainCollisionManager* GameState_Running::getCollisionManager()
	{
		return collisionMananger_.get();
	}
}