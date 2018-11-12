#pragma once
#include "Broadphase.h"
#include <SFML\Graphics.hpp>

#include <unordered_set>
#include <list>

namespace tgf
{
	namespace collision
	{
		struct object_info
		{
			collidingObject obj;
			// cells
			unsigned short x_min = 1, y_min = 1;
			unsigned short x_max = 0, y_max = 0;
		};

		struct cell
		{
			std::vector<collidingObject*> members;
		};

		// see https://stackoverflow.com/a/48355534
		class Broadphase_Grid : public Broadphase
		{
		public:
			Broadphase_Grid();
			~Broadphase_Grid();

			// Inherited via Broadphase
			virtual void update() override;
			virtual std::vector<std::pair<collidingObject*, collidingObject*>> findPairs() override;
			virtual std::vector<collidingObject*> getAllColliders() override;
			virtual std::vector<collidingObject*> findShapePairs(c2Shape * shape, uint16_t collision_mask) override;
			virtual void add(tgf::collision::collidingObject & obj) override;
			virtual void remove(CollisionEntity * obj) override;

			void calcGridCells(tgf::collision::c2Shape shape, sf::Vector2i & mincell, sf::Vector2i& maxcell);
			void initGridSize(unsigned short x, unsigned short y, float cellsize, sf::Vector2f pos = sf::Vector2f(0.0f, 0.0f));
			std::vector<collidingObject*> findPairs_forObject(collidingObject * collider, std::unordered_set<collidingObject*>& ignore_objects);
			std::vector<collidingObject*>& cellMembers(unsigned short x, unsigned short y);
			//void updateGrid();
		protected:
			void removeCollidingObjectFromCells(object_info& obj);

			std::list<object_info> colliders_;
			std::vector<cell> cells_;

			// grid dimensions
			sf::Vector2i	gridsize_;
			float			cellsize_;
			sf::Vector2f	posTopLeft_;
		};
	}
}