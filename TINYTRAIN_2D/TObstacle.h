#pragma once
#include "Entity.h"
#include <memory>

namespace tinytrain
{
	class TTrain;
	class GameState_Running;

	class TObstacle : public tgf::Entity
	{
	public:
		TObstacle(GameState_Running* gs, bool wintrigger = false);
		~TObstacle();

		// Inherited via Entity
		virtual void draw(sf::RenderTarget * target) override;
		virtual void update(float deltaTime) override;

		virtual void onTriggerEnter(Entity* a_other);

		const bool winningTrigger_;

		bool drawCollisionShape_;
		std::unique_ptr<sf::FloatRect> collisionShape_;

	protected:
		std::unique_ptr<sf::Drawable> drawable_;
		GameState_Running* gs_;
	};
}