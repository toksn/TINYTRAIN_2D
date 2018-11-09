#pragma once
#include "Broadphase.h"
#include <SFML\Graphics.hpp>

namespace tgf
{
	namespace collision
	{
		// see https://stackoverflow.com/a/48355534
		class Broadphase_Grid : public Broadphase
		{
		public:
			Broadphase_Grid();
			~Broadphase_Grid();

			// Inherited via Broadphase
			virtual void update() override;
			virtual std::vector<std::pair<collidingObject*, collidingObject*>> findPairs() override;
			virtual std::vector<collidingObject*> findShapePairs(c2Shape * shape, uint16_t collision_mask) override;
			virtual void add(tgf::collision::collidingObject & obj) override;
			virtual void remove(CollisionEntity * obj) override;			

			void initGridSize(unsigned short x, unsigned short y, float cellsize, sf::Vector2f pos = sf::Vector2f(0.0f, 0.0f));
			void updateGrid();
		protected:
			std::vector<collidingObject> colliders_;

			// grid dimensions
			sf::Vector2i broadGrid_size_;
			float broadGrid_cellsize_;
			sf::Vector2f broadGrid_posTopLeft_;

			std::vector<int> cells_;
		};
	}
}