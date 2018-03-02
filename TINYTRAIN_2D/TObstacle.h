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
		// todo: is there a workaround for this callback func typedef? 
		// it has to be redefined in every collisionentry derived class for now
		//typedef void(TObstacle::* collisionCallbackFunc)(tgf::Entity*);

		TObstacle(GameState_Running* gs, bool wintrigger = false);
		~TObstacle();

		// Inherited via CollisionEntity
		virtual void draw(sf::RenderTarget * target) override;
		virtual void update(float deltaTime) override;
		virtual tgf::collision::c2Shape getCollisionShape() override;
		virtual void updateCollisionShape() override;

		virtual void onTriggerEnter(Entity* a_other);
		const bool winningTrigger_;
		
		std::unique_ptr<sf::RectangleShape> drawable_;
		//bool drawCollisionShape_;
	protected:
		std::unique_ptr<c2Poly> collisionShape_;
		GameState_Running* gs_;
	};
}