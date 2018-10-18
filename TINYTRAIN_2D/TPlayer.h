#pragma once
#include "Entity.h"
#include "GameState_Running.h"
#include "TLineInputInterface.h"

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
		float input_angle_;
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

		bool addInputLineToRailTrack();
		std::vector<sf::Vector2f> convertLineToRailTrack(std::vector<sf::Vector2f>& line);
		std::vector<sf::Vector2f> castRailTrack(std::vector<sf::Vector2f>& line);

		TRailTrack* railtrack_;
		controllers::TLineInputInterface* input_component_;
		sf::Color color_;
		sf::RectangleShape drawingAreaShape_;

		// todo: move all the input dir stuff into railtrack to be worldcoordinates 
		//	OR	 calc the screenpos from end rail pos
		sf::VertexArray input_dir_;

		// auto cast feature
		bool autocast_;
		float railcast_maxcooldown_;
		float railcast_mincooldown_;
		float railcast_cooldown_;
		float railcast_timer_;
		bool first_rail_;
	};
}