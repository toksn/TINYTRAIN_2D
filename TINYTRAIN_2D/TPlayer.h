#pragma once
#include "Entity.h"
#include "GameState_Running.h"
#include "TPolyLineInputComponent.h"

namespace tinytrain
{
	enum class INPUTSTATE
	{
		IDLE,
		DRAWING_WAIT,
		DRAWING
	};

	class TRailTrack;

	class TPlayer : public tgf::Entity
	{
	public:
		TPlayer(GameState_Running* gs);
		~TPlayer();

		void recalcDrawRect(int width, int height);
		
		void setTrack(TRailTrack* track);

		//callbacks
		void onMousePressed(sf::Event& e);
		void onMouseReleased(sf::Event& e);
		void onKeyPressed(sf::Event & e);

		void setColor(sf::Color col);
		
		INPUTSTATE inputstate_;
		sf::FloatRect drawingArea_;
		GameState_Running* gs_;

		bool bNormalizeDrawnLineSize_;
		bool bNormalizeDrawnLineRotation_;
	protected:
		// Inherited via Entity
		virtual void onDraw(sf::RenderTarget * target) override;
		virtual void onUpdate(float deltaTime) override;


		void startDrawing(int x, int y);
		void stopDrawing();

		void addInputLineToRailTrack();
		std::vector<sf::Vector2f> convertLineToRailTrack(std::vector<sf::Vector2f>& line);

		TRailTrack* railtrack_;
		controllers::TPolyLineInputComponent* input_component_;
		sf::Color color_;
		sf::RectangleShape drawingAreaShape_;
	};
}