#pragma once
#include "Entity.h"

namespace tgf
{
	class Component
	{
		friend class Entity;
	public:
		virtual void draw(sf::RenderTarget * target) = 0;
		virtual void update(float deltaTime) = 0;

	protected:
		// make the owner a shared pointer? 
		// std::shared_ptr<Entity> owner_;
		// i think it is not needed in here because the components are always owned by
		// an entity with unique_ptr and therefore only exist while its owner is alive
		
		Entity* owner_;
		bool destroyed = false;
	};
}