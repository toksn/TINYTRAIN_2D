#pragma once
#include <SFML\Graphics.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

constexpr float RAD_TO_DEG = 180.0f / M_PI;
constexpr float DEG_TO_RAD = M_PI / 180.0f;

namespace tgf
{
	class Entity
	{
	public:
		// drawing this entity to the target. the caller (usally a gamestate) has to make sure the target is valid. no checks will be done in the entities!
		virtual void draw(sf::RenderTarget * target) = 0;
		virtual void update(float deltaTime) = 0;
	};
}

