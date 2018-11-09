#pragma once
#include "CollisionManager.h"
#include <SFML/Graphics.hpp>

namespace tgf
{
	namespace collision
	{
		// see https://stackoverflow.com/a/48355534
		class CollisionManagerGridBroadPhase : public CollisionManager
		{
		public:
			CollisionManagerGridBroadPhase();
			~CollisionManagerGridBroadPhase();

			virtual void update() override;
			void initGridSize(sf::Vector2i gridsize, float cellsize, sf::Vector2f pos = sf::Vector2f(0.0f, 0.0f));
			void updateGrid();
		protected:
			// grid dimensions
			sf::Vector2i broadGrid_size_;
			float broadGrid_cellsize_;
			sf::Vector2f broadGrid_posTopLeft_;

			std::vector<int> cells_;
		};
	}
}

