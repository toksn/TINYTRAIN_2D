#pragma once
#include "Entity.h"
#include <memory>

namespace tgf
{
	class Component
	{
		friend class Entity;
	public:
		virtual void draw(sf::RenderTarget * target) = 0;
		virtual void update(float deltaTime) = 0;
		virtual std::unique_ptr<Component> cloneComponent() = 0;

		//virtual void attachTo(Entity* entity);
		virtual std::unique_ptr<Component> detachFrom(Entity* entity);
		virtual std::unique_ptr<Component> detachFromOwner();

		bool drawDebug_;
	protected:
		//virtual void onDraw(sf::RenderTarget * target) = 0;
		//virtual void onUpdate(float deltaTime) = 0;

		// make the owner a shared pointer? 
		// std::shared_ptr<Entity> owner_;
		// i think it is not needed in here because the components are always owned by
		// an entity with unique_ptr and therefore only exist while its owner is alive
		
		Entity* owner_;
		bool destroyed_ = false;
	};
}