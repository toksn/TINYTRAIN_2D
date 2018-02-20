#pragma once
#include "Entity.h"

namespace tgf
{
	class GameStateBase;
}

namespace tinytrain
{
	enum class INPUTSTATE
	{
		IDLE,
		DRAWING
	};

	class TRailTrack;

	class TPlayer : tgf::Entity
	{
	public:
		TPlayer(tgf::GameStateBase* gs);
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

