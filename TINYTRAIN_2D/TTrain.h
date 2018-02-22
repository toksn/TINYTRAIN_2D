#pragma once
#include <SFML\Graphics.hpp>
#include "Entity.h"

namespace tinytrain
{
	class TTrain : public tgf::Entity
	{
	public:
		TTrain();
		~TTrain();

		void initWagons(const unsigned int a_numberOfWagons);

		// Inherited via Entity
		virtual void draw(sf::RenderTarget * target) override;
		virtual void update(const float dt) override;

		sf::Vector2f getPosition();

		// wagon stats
		float speed_;
		sf::Vector2f wagonsize_;
		float wagongap_;
		sf::Color color__firstwagon;
		sf::Color color__wagons;

		std::vector<sf::RectangleShape> wagons_;

		// distance travelled by the first wagon
		float distance_;

	};
}