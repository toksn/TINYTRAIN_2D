#pragma once
#include "tinyc2.h"
#include "CollisionEntity.h"
#include <memory>

namespace tinytrain
{
	class TTrain;
	class GameState_Running;

	class TCollisionZone : public tgf::collision::CollisionEntity
	{
	public:
		TCollisionZone(GameState_Running* gs, bool wintrigger = false, tgf::collision::CollisionManager::CollisionCategory cat = tgf::collision::CollisionManager::CollisionCategory::STATIC_CATEGORY_1);
		~TCollisionZone();

		void setCollisionShape(C2_TYPE type, void * shape);
		virtual tgf::collision::c2Shape getCollisionShape() override;
		void setCollisionCategory(tgf::collision::CollisionManager::CollisionCategory cat);

		virtual void onTriggerEnter(Entity* a_other);
		virtual void onTriggerLeave(Entity* a_other);
		
		const bool winningTrigger_;
		
		bool drawDebug_;
	protected:
		// Inherited via Entity
		virtual void onDraw(sf::RenderTarget * target) override;
		virtual void onUpdate(float deltaTime) override;

		tgf::collision::c2Shape collisionShape_;
		sf::VertexArray debugShape_;
		GameState_Running* gs_;
	};
}