#include "Entity.h"
#include <algorithm>

namespace tgf
{
	Entity::Entity()
	{
	}
	Entity::Entity(const Entity & other)
	{
		drawDebug_ = other.drawDebug_;
		for (auto& c : other.components_)
		{
			//components_.emplace_back(c->cloneComponent());
			// this ensures owner pointer of component to be set correctly
			addComponent(c->cloneComponent());
		}
	}

	// drawing this entity to the target. the caller (usally a gamestate) has to make sure the target is valid. no checks will be done in the entities!
	void Entity::draw(sf::RenderTarget * target)
	{
		for (auto& c : components_)
			c->draw(target);

		onDraw(target);
	}
	void Entity::update(float deltaTime)
	{
		for (auto& c : components_)
			c->update(deltaTime);

		// remove components that are marked as destroyed
		components_.erase( std::remove_if(components_.begin(), components_.end(), [](auto& c) { if (c->destroyed_) { printf("component removed.\n"); } return c->destroyed_; }), components_.end() );

		onUpdate(deltaTime);
	}

	void Entity::addComponent(std::unique_ptr<Component> a_component)
	{
		a_component->owner_ = this;
		components_.push_back(std::move(a_component));
	}

	void Entity::removeComponent(Component * a_component)
	{
		// just set destroyed value and wait for the update loop to clear it
		for (auto& c : components_)
		{
			if (c.get() == a_component)
			{
				c->destroyed_ = true;
				return;
			}
		}

		/*// less safe: delete it right here
		for (auto c = components_.begin(); c != components_.end(); ++c)
		{
			components_.erase(c);
			return;
		}*/
	}
	std::unique_ptr<Component> Entity::detachComponent(Component * a_component)
	{
		auto comp = std::unique_ptr<Component>();
		
		for (auto c = components_.begin(); c != components_.end(); ++c)
		{
			comp = std::move(*c);
			components_.erase(c);
			break;
		}

		return comp;
	}
}