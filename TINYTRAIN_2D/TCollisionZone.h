#pragma once
#include "tinyc2.h"
#include "CollisionEntity.h"
//#include "CollisionManager.h"
#include <memory>

namespace tinytrain
{
	class TTrain;
	class GameState_Running;

	class TCollisionZone : public tgf::collision::CollisionEntity
	{
	public:
		TCollisionZone(GameState_Running* gs, bool wintrigger = false, tgf::collision::CollisionCategory cat = tgf::collision::CollisionCategory::STATIC_CATEGORY_1);
		//TCollisionZone(const tinytrain::TCollisionZone &other);
		~TCollisionZone();

		void setCollisionCategory(tgf::collision::CollisionCategory cat);
		void setCollisionShape_AABB(c2v min, c2v max);
		void setCollisionShape_Poly(const c2Poly poly);
		void updateCollisionShape_matchDrawable();

		virtual void onTriggerEnter(tgf::collision::CollisionEntity* a_other);
		virtual void onTriggerLeave(tgf::collision::CollisionEntity* a_other);
				
		const bool winningTrigger_;
		
		bool drawDebug_;
		std::unique_ptr<sf::RectangleShape> drawable_;
		bool drawableMoved_;
	protected:
		// Inherited via Entity
		virtual void onDraw(sf::RenderTarget * target) override;
		virtual void onUpdate(float deltaTime) override;

		sf::VertexArray debugShape_;
		GameState_Running* gs_;

		// TODO: c++17 std::variant
		c2AABB aabb_shape_;
		c2Poly poly_shape_;
	};
}