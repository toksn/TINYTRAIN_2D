#pragma once
#include <SFML\Graphics.hpp>
#include "Entity.h"

namespace tinytrain
{
	class GameState_Running;

	class TTrain : public tgf::Entity
	{
	public:
		TTrain(GameState_Running* gs);
		~TTrain();

		void initWagons(const unsigned int a_numberOfWagons);

		sf::Vector2f getPosition();

		// wagon stats
		float speed_;
		sf::Vector2f wagonsize_;
		float wagongap_;
		sf::Color color_firstwagon_;
		sf::Color color_wagons_;

		std::vector<sf::RectangleShape> wagons_;

		// distance travelled by the first wagon
		float distance_;

		void play();
		void pause();		
		void reset();
		void stop();

		sf::FloatRect getAABB();
		bool debugDraw_;

	protected:
		// Inherited via Entity
		virtual void onDraw(sf::RenderTarget * target) override;
		virtual void onUpdate(const float dt) override;

		void calcAABB();
		//sf::FloatRect aabb_;
		sf::VertexArray aabb_;

		GameState_Running* gs_;
		bool isRunning_;
	};
}