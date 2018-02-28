#pragma once
#include "Entity.h"

namespace tinytrain
{
	enum class INPUTSTATE
	{
		IDLE,
		DRAWING
	};

	class TRailTrack;
	class GameState_Running;

	class TPlayer : tgf::Entity
	{
	public:
		TPlayer(GameState_Running* gs);
		~TPlayer();

		// Inherited via Entity
		virtual void draw(sf::RenderTarget * target) override;
		virtual void update(float deltaTime) override;

		void recalcDrawRect(int width, int height);
		void setTrack(TRailTrack* track);

		//callbacks
		void onMousePressed(sf::Event& e);
		void onMouseReleased(sf::Event& e);
		void onKeyPressed(sf::Event & e);

		void setColor(sf::Color col);
		
	private:
		void stopDrawing();
		void addDrawnLineToRailTrack();

		TRailTrack* railtrack_;
		GameState_Running* gs_;
		sf::Color color_;

		INPUTSTATE inputstate_;
		sf::FloatRect drawingArea_;
		sf::RectangleShape drawingAreaShape_;

		sf::VertexArray drawnLine_;
		float minDist_;
	};
}

