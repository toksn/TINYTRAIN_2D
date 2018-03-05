#include "Entity.h"
#include <algorithm>

namespace tgf
{
	// drawing this entity to the target. the caller (usally a gamestate) has to make sure the target is valid. no checks will be done in the entities!
	void Entity::draw(sf::RenderTarget * target)
	{
		for (auto& c : components_)
			c->draw(target);
	}
	void Entity::update(float deltaTime)
	{
		for (auto& c : components_)
			c->update(deltaTime);

		// remove components that are marked as destroyed
		components_.erase(std::remove_if(components_.begin(), components_.end(), [](auto& c){ return c->destroyed; }));
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
				c->destroyed = true;
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