#include "tinyc2.h"
#include "TLevel.h"
#include "TTrain.h"
#include "TRailTrack.h"
#include "TObstacle.h"
#include "InterpolateToPoint.h"
#include "SplineTexture.h"
#include "GameState_Running.h"
#include "TCollisionZone.h"
#include "TCar.h"

namespace tinytrain
{
	TLevel::TLevel(GameState_Running * gs)
	{
		gs_ = gs;

		elapsed_time_ = 0.0f;
		points_ = 0;

		background_static_.setPrimitiveType(sf::PrimitiveType::Quads);
		foreground_static_.setPrimitiveType(sf::PrimitiveType::Quads);
		foreground_static2_.setPrimitiveType(sf::PrimitiveType::Quads);
		foreground_dynamic_.setPrimitiveType(sf::PrimitiveType::Quads);

		if (gs_ && gs_->game_)
			texture_atlas_ = gs_->game_->getTextureAtlas();

		drawDebug_ = false;
		if (gs_)
		{
			gs_->bindEventCallback(sf::Event::KeyPressed, this, &TLevel::onKeyPressed);
			//...
		}
	}

	//TLevel::TLevel(GameState_Running* gs, const level_info & info) : TLevel(gs)
	//{
	//	info_ = info;
	//}


	TLevel::~TLevel()
	{
		if (gs_)
			gs_->unbindAllCallbacks(this);
	}

	void TLevel::onDraw(sf::RenderTarget * target)
	{
		auto renderstate = sf::RenderStates::Default;
		if(texture_atlas_)
			renderstate = sf::RenderStates::RenderStates(texture_atlas_->getTexture());
		
		target->draw(background_static_, renderstate);
		
		target->draw(roads_, renderstate);
		if (drawDebug_)
		{
			target->draw(roads_debug_);
		}

		if (railtrack_)
			railtrack_->draw(target);
		if (train_)
			train_->draw(target);
		
		target->draw(foreground_static2_, renderstate);
		target->draw(foreground_static_, renderstate);
		target->draw(foreground_dynamic_, renderstate);

		for (int i = obstacles_.size() - 1; i >= 0; i--)
		{
			auto o = obstacles_[i].get();
			if (o)
				o->draw(target);
			else
				obstacles_.erase(obstacles_.begin() + i);
		}

		for (auto& c : static_collision_)
			c->draw(target);
	}

	void TLevel::onUpdate(float deltaTime)
	{
		elapsed_time_ += deltaTime;

		if (train_)
			train_->update(deltaTime);
		if (railtrack_)
			railtrack_->update(deltaTime);

		road_network_.update(deltaTime);

		for (int i = obstacles_.size()-1; i >= 0; i--)
		{
			auto o = obstacles_[i].get();
			if (o)
				o->update(deltaTime);
			else
				obstacles_.erase(obstacles_.begin()+i);
		}	

		for (auto& c : static_collision_)
			c->update(deltaTime);


		if (elapsed_time_ >= info_.timelimit)
		{
			elapsed_time_ = info_.timelimit;
			gs_->lost(train_.get());
		}
	}

	void TLevel::onKeyPressed(sf::Event& e)
	{
		// PAUSE
		if (e.key.code == DEBUG_KEY)
		{
			// toggle debug drawing
			drawDebug_ = !drawDebug_;

			for (auto& o : obstacles_)
			{
				auto car = dynamic_cast<TCar*>(o.get());
				if (car != nullptr)
				{
					car->drawDebug_ = drawDebug_;
					for (auto& comp : car->components_)
						comp->drawDebug_ = drawDebug_;
				}					
			}

			for (auto& c : static_collision_)
				c->drawDebug_ = drawDebug_;

			train_->debugDraw_ = drawDebug_;
		}
	}
	
	void TLevel::restart_()
	{
		// TODO: implement fast restart without reloading the level
	}
}