#pragma once
#include <memory>
#include "Entity.h"
#include "TRailTrack.h"

namespace tinytrain
{
	class TTrain;

	class TLevel : tgf::Entity
	{
	public:
		TLevel();
		~TLevel();

		// Inherited via Entity
		virtual void draw(sf::RenderTarget * target) override;
		virtual void update(float deltaTime) override;

		// load a level from file
		void load(std::string file = "");
		
		std::unique_ptr<TTrain> train_;
		std::unique_ptr<TRailTrack> railtrack_;
	};


}