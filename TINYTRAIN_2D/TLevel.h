#pragma once
#include <memory>
#include "Entity.h"
#include "TRailTrack.h"

namespace tinytrain
{
	class TTrain;
	class TObstacle;
	class GameState_Running;

	class TLevel : tgf::Entity
	{
	public:
		TLevel();
		~TLevel();

		// Inherited via Entity
		virtual void draw(sf::RenderTarget * target) override;
		virtual void update(float deltaTime) override;

		// load a level from file
		void load(GameState_Running* gs, std::string file = "");

		void restart(GameState_Running* gs);
		
		std::unique_ptr<TTrain> train_;
		std::unique_ptr<TRailTrack> railtrack_;
		std::vector<std::unique_ptr<TObstacle>> obstacles_;
	};


}