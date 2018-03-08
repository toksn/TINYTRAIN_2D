#pragma once
#include <SFML\Graphics.hpp>
#include "Component.h"
#include <memory>

#define _USE_MATH_DEFINES
#include <math.h>

constexpr float RAD_TO_DEG = 180.0f / M_PI;
constexpr float DEG_TO_RAD = M_PI / 180.0f;

namespace tgf
{
	class Component;

	class Entity : public sf::Transformable
	{
	public:
		// drawing this entity to the target. the caller (usally a gamestate) has to make sure the target is valid. no checks will be done in the entities!
		virtual void draw(sf::RenderTarget * target);
		virtual void update(float deltaTime);

		virtual void addComponent(std::unique_ptr<Component> a_component);
		virtual void removeComponent(Component* a_component);
		virtual std::unique_ptr<Component> detachComponent(Component* a_component);

		template<typename T, typename... TArgs> inline T* addNewComponent(TArgs&&... args)
		{
			static_assert(std::is_base_of<Component, T>::value, "Object to add must be derived from 'Component'");
			// create unique pointer by forwarding the given arguments + put it in the components vector
			components_.emplace_back(std::make_unique<T>(std::forward<TArgs>(args)...));
			auto uptr = (*components_.rbegin());
			uptr->owner_ = this;

			/* TODO: components grouped by type not needed yet. may be needed in the furture
			auto ptr = (*components_.rbegin()).get();
			// create a hash entry for the typeid(T) and put the raw pointer in there
			components_grouped_[typeid(T).hash_code()].emplace_back(ptr);*/
			
			// return raw pointer
			return uptr.get();
		}

	protected:
		//std::vector<std::unique_ptr<Component>> components_removed;
		std::vector<std::unique_ptr<Component>> components_;
	};
}

