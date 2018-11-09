#pragma once
#include <SFML\Graphics.hpp>
#include "CollisionEntity.h"
#include "TPassenger.h"

namespace tinytrain
{
	class GameState_Running;

	class TTrain : public tgf::collision::CollisionEntity
	{
	public:
		TTrain(GameState_Running* gs);
		~TTrain();

		void initWagons(const unsigned int a_numberOfWagons);

		bool hasCapacity(unsigned int space_needed = 1);
		void pickUp(std::unique_ptr<TPassenger> passenger);

		sf::Vector2f getPosition();

		// wagon stats
		float speed_;
		sf::Vector2f wagonsize_;
		float wagongap_;
		sf::Color color_firstwagon_;
		sf::Color color_wagons_;
		sf::Color color_filledwagons_;

		std::vector<sf::RectangleShape> wagons_;
		std::vector<std::unique_ptr<TPassenger>> passengers_;
		std::vector<std::unique_ptr<TPassenger>> passengers_done_;

		// distance travelled by the first wagon
		float distance_;

		void play();
		void pause();		
		void reset();
		void stop();

		sf::FloatRect getAABB();
		bool debugDraw_;

		void collision(Entity* other);
		void collisionEnd(Entity* other);

	protected:
		// Inherited via Entity
		virtual void onDraw(sf::RenderTarget * target) override;
		virtual void onUpdate(const float dt) override;

		void calcAABB();
		//sf::FloatRect aabb_;
		sf::VertexArray aabb_;
		// helper variable to keep from updating the colors every tick
		unsigned int passenger_count_;

		GameState_Running* gs_;
		bool isRunning_;
	};
}