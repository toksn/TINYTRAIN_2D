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
		void onMousePressed(sf::Event e);

	private:
		TRailTrack* m_railtrack;

		INPUTSTATE m_inputstate;
		sf::FloatRect m_drawingArea;
		sf::RectangleShape m_drawingAreaShape;
	};
}

