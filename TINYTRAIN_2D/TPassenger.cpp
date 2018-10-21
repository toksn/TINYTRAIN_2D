#include "TPassenger.h"
#include "GameState_Running.h"
#include "TRoadNavComponent.h"
#include "TLevel.h"

namespace tinytrain
{
	TPassenger::TPassenger(GameState_Running * gs) : TObstacle(gs, true)
	{
		collision_quad_.setPrimitiveType(sf::PrimitiveType::LineStrip);
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Yellow));
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Yellow));
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Yellow));
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Yellow));
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Yellow));

		sf::RectangleShape rect(sf::Vector2f(20.0f, 20.0f));
		destination_drawable_ = std::make_unique<sf::RectangleShape>(rect);

		points_ = 1;
		state_ = PassengerState::WAIT_FOR_PICKUP;
	}
	
	TPassenger::~TPassenger()
	{
	}

	void TPassenger::reset()
	{
		setState(PassengerState::WAIT_FOR_PICKUP);
	}

	void TPassenger::setState(TPassenger::PassengerState newstate)
	{
		if (state_ != newstate)
		{
			drawable_.swap(destination_drawable_);

			updateCollisionShape();

			state_ = newstate;
		}

		//if (newstate == PassengerState::COLLECTED)
		//{
		//	
		//}
	}

	TPassenger::PassengerState TPassenger::getState()
	{
		return state_;
	}

	void TPassenger::onDraw(sf::RenderTarget * target)
	{
		TObstacle::onDraw(target);

		if(drawDebug_)
			target->draw(collision_quad_);
	}

	void TPassenger::onUpdate(float deltaTime)
	{
		TObstacle::onUpdate(deltaTime);		
	}


	void TPassenger::onTriggerEnter(Entity * other)
	{
	}
}
