#pragma once
#include "TObstacle.h"

namespace tinytrain
{
	class TLevel;
	class TPassenger : public TObstacle
	{
	public:
		TPassenger(GameState_Running* gs);
		~TPassenger();

		void reset();

		unsigned int id_;
		TLevel*		 level_;
		enum class PassengerState
		{
		//	NORMAL,					// normal moving
			WAIT_FOR_PICKUP,		// waiting for a train
			COLLECTED				// collected by a train
		};

		//sf::IntRect start_coords_;
		//sf::IntRect destination_coords_;
		std::unique_ptr<sf::RectangleShape> destination_drawable_;
		unsigned int points_;

		void setState(PassengerState newstate);
		PassengerState getState();

	private:
		PassengerState state_;

		virtual void onDraw(sf::RenderTarget * target) override;
		virtual void onUpdate(float deltaTime) override;

		virtual void onTriggerEnter(tgf::collision::CollisionEntity* other) override;

		sf::VertexArray collision_quad_;
	};
}