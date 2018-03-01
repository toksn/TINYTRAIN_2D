#pragma once
#include <memory>
#include "GameStateBase.h"
#include "TTrainCollisionManager.h"

namespace tgf
{
	class Entity; 
}

namespace tinytrain
{
	class TLevel;
	class TPlayer;
	class TTrain;

	class GameState_Running : public tgf::GameStateBase
	{
	public:
		GameState_Running(tgf::Game* game);
		~GameState_Running();

		// Inherited via GameStateBase
		virtual void update(float deltaTime) override;
		virtual void draw(sf::RenderTarget * target) override;
		//virtual void handleInput(sf::Event& e) override;
		virtual void onWindowSizeChanged(int w, int h) override;
		
		void moveCameraToLastRail();
		void moveCameraToPoint(sf::Vector2f pos, float angle, float time);

		void won(TTrain* train);
		void lost(TTrain* train);
		void pause();
		void restart();

		TTrainCollisionManager* getCollisionManager();

	protected:
		void initCurrentLevel();

		std::unique_ptr<TLevel> level_;
		std::unique_ptr<TPlayer> player_;
		std::unique_ptr<TTrainCollisionManager> collisionMananger_;

		// camera related
		std::unique_ptr<sf::View> camera_;
		bool bRotateCameraWithTrack_;
		float camFlowTime_;
		float camCurrentTime_;
		sf::Vector2f camNewPos_;
		sf::Vector2f camOldPos_;
		float camNewRot_;
		float camOldRot_;
	};
}