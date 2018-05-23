#include "tinyc2.h"
#include "TLevel.h"
#include "TTrain.h"
#include "TRailTrack.h"
#include "TObstacle.h"
#include "InterpolateToPoint.h"
#include "SplineTexture.h"
#include "GameState_Running.h"

namespace tinytrain
{
	TLevel::TLevel(GameState_Running* gs)
	{
		//graph g;
		//g.addEdge(1, 2, 34.212f);
		gs_ = gs;

		background_static.setPrimitiveType(sf::PrimitiveType::Quads);
		foreground_static.setPrimitiveType(sf::PrimitiveType::Quads);
		foreground_dynamic.setPrimitiveType(sf::PrimitiveType::Quads);
			

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
		target->draw(background_static, sf::RenderStates::RenderStates(texture_atlas_->getTexture()));
		
		target->draw(roads_, sf::RenderStates::RenderStates(texture_atlas_->getTexture()));
		if(drawDebug_)
			target->draw(roads_debug_);
		

		if (railtrack_)
			railtrack_->draw(target);
		if (train_)
			train_->draw(target);
		
		target->draw(foreground_static, sf::RenderStates::RenderStates(texture_atlas_->getTexture()));
		target->draw(foreground_dynamic, sf::RenderStates::RenderStates(texture_atlas_->getTexture()));

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
	
	void TLevel::restart_()
	{
		// TODO: implement fast restart without reloading the level
	}
}