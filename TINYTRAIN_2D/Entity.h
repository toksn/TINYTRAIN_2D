#pragma once
#include <SFML\Graphics.hpp>

namespace tgf
{
	class Entity
	{
	public:
		// drawing this entity to the target. the caller (usally a gamestate) has to make sure the target is valid. no checks will be done in the entities!
		virtual void draw(sf::RenderWindow* target) = 0;
		virtual void update(float deltaTime) = 0;
	};
}

