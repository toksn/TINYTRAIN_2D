#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <SFML\Graphics.hpp>

enum direction
{
	NORTH = 0,	// 0
	EAST,		// 1
	SOUTH,		// 2
	WEST,		// 3
	DIR_COUNT	// 4
};

enum class directionMode
{
	OneWay = 0,		// A>B   (A>B)  (A>B)
	TwoWay			// A>B>A (A>B>A) (A>B>A)
};


constexpr float RAD_TO_DEG = 180.0f / M_PI;
constexpr float DEG_TO_RAD = M_PI / 180.0f;


template<class T> struct vec2Less {
	bool operator()(const sf::Vector2<T> & a, const sf::Vector2<T>& b) const {
		if (a.x != b.x)
			return a.x < b.x;
		else
			return a.y < b.y;
	}
};