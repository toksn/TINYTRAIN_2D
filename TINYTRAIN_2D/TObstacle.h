#pragma once
#include "tinyc2.h"
#include "CollisionEntity.h"
#include <memory>

namespace tinytrain
{
	class TTrain;
	class GameState_Running;

	class TObstacle : public tgf::collision::CollisionEntity
	{
	public:
		TObstacle(GameState_Running* gs, bool wintrigger = false);
		~TObstacle();

		virtual tgf::collision::c2Shape getCollisionShape() override;
		virtual void updateCollisionShape();

		virtual void onTriggerEnter(CollisionEntity* a_other);
		const bool winningTrigger_;
		
		std::unique_ptr<sf::RectangleShape> drawable_;
		//bool drawCollisionShape_;
	protected:
		// Inherited via CollisionEntity
		virtual void onDraw(sf::RenderTarget * target) override;
		virtual void onUpdate(float deltaTime) override;

		std::unique_ptr<c2Poly> poly_;
		GameState_Running* gs_;
	};
}