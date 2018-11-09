#include "TCar.h"
#include "GameState_Running.h"
#include "TRoadNavComponent.h"

namespace tinytrain
{
	TCar::TCar(GameState_Running * gs, bool wintrigger) : TObstacle(gs, wintrigger)
	{
		collision_quad_.setPrimitiveType(sf::PrimitiveType::LineStrip);
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Blue));
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Blue));
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Blue));
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Blue));
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Blue));

		waitInTraffic_checkingInterval_ = 0.35f;
		state_ = DrivingState::NORMAL;
	}

	TCar::~TCar()
	{
	}

	void TCar::onDraw(sf::RenderTarget * target)
	{
		TObstacle::onDraw(target);

		if(drawDebug_)
			target->draw(collision_quad_);
	}

	void TCar::onUpdate(float deltaTime)
	{
		TObstacle::onUpdate(deltaTime);
		
		//if (state_ == DrivingState::NORMAL || state_ == DrivingState::WAIT_IN_TRAFFIC)
		if(navi_ && navi_->getState() != components::TRoadNavComponent::NavState::RUNNING_WAIT_FOR_CLEAR_ROAD)
		{
			
			if (state_ == DrivingState::WAIT_IN_TRAFFIC)
			{
				drawable_->setFillColor(sf::Color::Red);
				timeSinceLastCheck_ += deltaTime;
				if (timeSinceLastCheck_ < waitInTraffic_checkingInterval_)
				{
					return;
				}

				timeSinceLastCheck_ = 0.0f;
			}
			else
				drawable_->setFillColor(sf::Color::Green);
			

			// check in front of the car
			tgf::collision::c2Shape s;

			sf::Vector2f size = drawable_->getSize();
			sf::Vector2f origin = drawable_->getOrigin();
			size.y *= 0.4f;
			size.x *= 1.1f;
			size.x -= origin.x;

#if 0
			//AABB checking	 
			s.type_ = C2_TYPE::C2_AABB;
			c2AABB shape_tocheck;
			sf::Vector2f midPointToCheck = getTransform().transformPoint(size.x + size.y, 0.0f);
			shape_tocheck.min = { midPointToCheck.x - size.y, midPointToCheck.y - size.y };
			shape_tocheck.max = { midPointToCheck.x + size.y, midPointToCheck.y + size.y };
			s.shape_ = &shape_tocheck;
			collision_quad_[0].position.x = shape_tocheck.min.x;
			collision_quad_[0].position.y = shape_tocheck.min.y;
			collision_quad_[1].position.x = shape_tocheck.max.x;
			collision_quad_[1].position.y = shape_tocheck.min.y;
			collision_quad_[2].position.x = shape_tocheck.max.x;
			collision_quad_[2].position.y = shape_tocheck.max.y;
			collision_quad_[3].position.x = shape_tocheck.min.x;
			collision_quad_[3].position.y = shape_tocheck.max.y;
			collision_quad_[4] = collision_quad_[0];
#else	
			// exact c2Poly matching (no AABB because rotation)
			s.type_ = C2_TYPE::C2_POLY;
			c2Poly shape_tocheck;
			collision_quad_[0] = getTransform().transformPoint(size.x, -size.y);
			collision_quad_[1] = getTransform().transformPoint(size.x, size.y);
			collision_quad_[2] = getTransform().transformPoint(size.x + size.y + size.y, size.y);
			collision_quad_[3] = getTransform().transformPoint(size.x + size.y + size.y, -size.y);
			collision_quad_[4] = collision_quad_[0];

			shape_tocheck.verts[0] = { collision_quad_[0].position.x, collision_quad_[0].position.y };
			shape_tocheck.verts[1] = { collision_quad_[1].position.x, collision_quad_[1].position.y };
			shape_tocheck.verts[2] = { collision_quad_[2].position.x, collision_quad_[2].position.y };
			shape_tocheck.verts[3] = { collision_quad_[3].position.x, collision_quad_[3].position.y };
			shape_tocheck.count = 4;
#endif			
			s.shape_ = &shape_tocheck;
			unsigned int mask = 0;
			mask |= tgf::collision::CollisionCategory::DYNAMIC_CATEGORY_1;
			if (gs_->getCollisionManager()->checkShapeForCollisions(&s, mask))
			{
				state_ = DrivingState::WAIT_IN_TRAFFIC;
				navi_->speed_ = 0.0f;
			}
			else
			{
				state_ = DrivingState::NORMAL;
				navi_->speed_ = vmax_;
			}
		}
		else
			drawable_->setFillColor(sf::Color::Cyan);

		//if(state_ == DrivingState::NORMAL)
		if (navi_)
			if (navi_->getState() == components::TRoadNavComponent::NavState::RUNNING_ || navi_->getState() == components::TRoadNavComponent::NavState::RUNNING_ON_CROSSING)
				//drawable_moved_ = true;
				collisionUpdated = true;
	}
	
	void TCar::onTriggerEnter(tgf::collision::CollisionEntity * other)
	{
	}
}
