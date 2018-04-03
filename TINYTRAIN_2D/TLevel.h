#pragma once
#include <memory>
#include "Entity.h"
#include "TRailTrack.h"
#include "CityGenerator.h"

namespace tinytrain
{
	class TTrain;
	class TObstacle;
	class GameState_Running;

	class TLevel : public tgf::Entity
	{
	public:
		TLevel();
		~TLevel();

		// load a level from file
		void load(GameState_Running* gs, std::string file = "");

		void restart(GameState_Running* gs);
				
		std::unique_ptr<TTrain> train_;
		std::unique_ptr<TRailTrack> railtrack_;
		std::vector<std::unique_ptr<TObstacle>> obstacles_;

	protected:
		// Inherited via Entity
		virtual void onDraw(sf::RenderTarget * target) override;
		virtual void onUpdate(float deltaTime) override;

		sf::VertexArray roads_;
		sf::VertexArray roads_debug_;

		sf::VertexArray triangulateRoadSegments(tgf::utilities::CityGenerator & city);
	};


}