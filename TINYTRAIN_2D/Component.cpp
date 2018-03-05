#include "Component.h"

namespace tgf
{
	std::unique_ptr<Component> Component::detachFrom(Entity * entity)
	{
		if (entity != nullptr)
			return entity->detachComponent(this);

		return std::unique_ptr<Component>();
	}

	std::unique_ptr<Component> Component::detachFromOwner()
	{
		return detachFrom(owner_);
	}
}
