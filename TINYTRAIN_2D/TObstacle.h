#pragma once
#include "Entity.h"
#include "tinyc2.h"
#include "TTrainCollisionManager.h"
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

		c2Shape getCollisionShape();
		void updateCollisionShape();

		const bool winningTrigger_;
		//bool drawCollisionShape_;
	protected:
		std::unique_ptr<sf::RectangleShape> drawable_;
		std::unique_ptr<c2Poly> collisionShape_;
		GameState_Running* gs_;
	};
}