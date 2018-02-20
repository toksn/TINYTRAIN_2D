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

		void setColor(sf::Color col);
	private:
		void stopDrawing();

		TRailTrack* m_railtrack;
		GameState_Running* m_gs;
		sf::Color m_color;

		INPUTSTATE m_inputstate;
		sf::FloatRect m_drawingArea;
		sf::RectangleShape m_drawingAreaShape;

		sf::VertexArray m_drawnLine;
		float m_minDist;
	};
}

