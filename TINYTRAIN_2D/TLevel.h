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
		TLevel(GameState_Running* gs);
		~TLevel();

		// load a level from file
		void load(std::string file = "");

		void restart();
				
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
		std::unique_ptr<sf::Texture> tex_;
		GameState_Running* gs_;
	};
}