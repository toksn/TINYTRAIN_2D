#include "tinyc2.h"
#include "TLevel.h"
#include "TTrain.h"
#include "TRailTrack.h"
#include "TObstacle.h"
#include "InterpolateToPoint.h"
#include "SplineTexture.h"
#include "GameState_Running.h"
#include "TCollisionZone.h"
#include "TPassenger.h"

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
		inworld_imgs_.setPrimitiveType(sf::PrimitiveType::Quads);

		if (gs_ && gs_->game_)
			texture_atlas_ = gs_->game_->getTextureAtlas();

		drawDebug_ = false;
		if (gs_)
		{
			gs_->bindEventCallback(sf::Event::KeyPressed, this, &TLevel::onKeyPressed);
			//...
		}

		//arrow_tx_.loadFromFile("data/images/arrow.png");
		// fill arrow to copy from for resizing the arrows_ array
		if (texture_atlas_)
		{
			arrow_.setTexture(*texture_atlas_->getTexture());
			auto area = texture_atlas_->getArea("arrow");
			arrow_.setTextureRect(area);
			arrow_.setOrigin(area.width * 0.5f, area.height * 0.5f);
			
			arrow_.setScale(0.5f, 0.5f);
			// radius at which the arrows are displayed (from the train)
			arrow_radius_ = 64.0f;
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

		target->draw(inworld_imgs_, renderstate);

		for (auto& p : passengers_)
			p->draw(target);

		for (auto& tz : targetzones_)
			tz->draw(target);

		for (auto& c : static_collision_)
			c->draw(target);

		for (auto& a : arrows_)
			target->draw(a);
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

		for (auto& p : passengers_)
			p->update(deltaTime);

		if (targetzones_.empty() && points_ >= info_.points_to_reach)
		{
			// use end point to create target zone
			//c++17 for (auto& [placement_rect, texture_rect] : level->info_.stations)
			for (auto& e : info_.stations)
			{
				auto placement_rect = e.first;
				auto texture_rect = e.second;

				auto zone = std::make_unique<TObstacle>(gs_, true);
				zone->drawable_->setPosition(placement_rect.left, placement_rect.top);
				zone->drawable_->setSize(sf::Vector2f(placement_rect.width, placement_rect.height));
				zone->drawable_->setOrigin(0.0f, 0.0f);
				zone->drawable_->setFillColor(sf::Color(100, 180, 0, 100));
				zone->drawable_->setOutlineColor(sf::Color(100, 210, 0, 200));
				zone->drawable_->setOutlineThickness(3.0f /** background_size_factor*/);
				zone->updateCollisionShape();

				if (texture_rect.width != 0 && texture_rect.height != 0)
				{
					// todo: obstacle set texture to replace colored area
					//zone->setTexture(texture_atlas_, texture_rect);
				}

				targetzones_.push_back(std::move(zone));
			}
		}

		for (auto& c : static_collision_)
			c->update(deltaTime);


		if (info_.timelimit > 0 && elapsed_time_ >= info_.timelimit)
		{
			elapsed_time_ = info_.timelimit;
			gs_->lost(train_.get());
		}

		// draw arrows from train to passengers / destinations
		if (train_)
		{
			sf::Vector2f pos = train_->getPosition();
			if (targetzones_.size())
			{
				arrows_.resize(targetzones_.size(), arrow_);

				for (int i = 0; i < targetzones_.size(); i++)
				{
					auto& a = arrows_[i];
					auto& t = targetzones_[i];
					
					sf::Vector2f targetpos = t->drawable_->getPosition() + t->drawable_->getSize() / 2.0f;

					placeArrow(a, pos, targetpos, sf::Color(100, 180, 0, 200), 0.6f);
				}
			}
			else
			{
				int size = train_->passengers_.size();
				int i = 0;
				//arr.resize(size * 2);

				if (train_->hasCapacity())
				{
					size += passengers_.size();
					arrows_.resize(size, arrow_);

					for (auto& p : passengers_)
					{
						auto& a = arrows_[i];

						sf::Vector2f targetpos = p->drawable_->getPosition() + p->drawable_->getSize() / 2.0f;

						placeArrow(a, pos, targetpos, sf::Color(180, 180, 0, 200));

						i++;
					}
				}
				
				arrows_.resize(size, arrow_);
				for (auto& p : train_->passengers_)
				{
					auto& a = arrows_[i];

					sf::Vector2f targetpos = p->drawable_->getPosition() + p->drawable_->getSize() / 2.0f;

					placeArrow(a, pos, targetpos, sf::Color(255, 100, 0, 200));

					i++;
				}
			}			
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
				auto car = dynamic_cast<TPassenger*>(o.get());
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

	void TLevel::placeArrow(sf::Sprite& a, sf::Vector2f sourcepos, sf::Vector2f targetpos, sf::Color col, float min_alpha_factor)
	{
		// direction
		sf::Vector2f dir = targetpos - sourcepos;
		//dir = tgf::math::MathHelper2D::normalizeVector(dir);
		float dist = sqrt(dir.x * dir.x + dir.y*dir.y);
		dir /= dist;

		if (dist < arrow_radius_)
			dir *= dist * 0.9f;
		else
			dir *= arrow_radius_;

		// angle
		float angle = atan2(dir.y, dir.x);
		angle *= RAD_TO_DEG;
		angle += 90.0f;

		// color
		constexpr float mindist = 64.0f * background_size_factor * 3.5f;
		constexpr float maxdist = 64.0f * background_size_factor * 8.0f;
		float colfactor = 1.0f;
		if (dist > mindist)
		{
			colfactor = 1.0f - (dist - mindist) / maxdist;
			colfactor = c2Max(colfactor, min_alpha_factor);
			//if (dist >= maxdist+mindist)
			//	colfactor = 0.0f;
			//else
				
		}
		col.a = 255.0f * colfactor;


		a.setPosition(sourcepos + dir);
		a.setRotation(angle);
		a.setColor(col);
	}
	
	void TLevel::restart_()
	{
		// TODO: implement fast restart without reloading the level
	}

	std::unique_ptr<TPassenger> TLevel::removePassenger(unsigned int id)
	{
		// find passenger element with given (passenger)id
		auto it = std::find_if(passengers_.begin(), passengers_.end(), [&id](auto& current)
		{
			return current->id_ == id;
		});

		if (it != passengers_.end())
		{
			std::unique_ptr<TPassenger> result{ static_cast<TPassenger*>((*it).release()) };
			passengers_.erase(it);
			return std::move(result);
		}
		return nullptr;
	}

	void TLevel::addPassenger(std::unique_ptr<TPassenger> newpass)
	{
		if (newpass != nullptr)
		{
			newpass->id_ = passenger_id_++;
			newpass->level_ = this;
			passengers_.push_back(std::move(newpass));
		}
	}
}