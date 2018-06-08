#pragma once
#include "TObstacle.h"

namespace tinytrain
{
	namespace components
	{
		class TRoadNavComponent;
	}
	

	class TCar : public TObstacle
	{
	public:
		TCar(GameState_Running* gs, bool wintrigger = false);
		~TCar();


		enum class DrivingState
		{
			NORMAL,					// normal driving
			WAIT_FREE_ROAD,			// waiting at a stopping point for rects to check to be free
			WAIT_IN_TRAFFIC			// wait behind another car, road checking frequency can be toned down
		};

		float vmax_;
		components::TRoadNavComponent* navi_;
		bool drawDebug_;
	private:
		virtual void onDraw(sf::RenderTarget * target) override;
		virtual void onUpdate(float deltaTime) override;

		sf::VertexArray collision_quad_;

		DrivingState state_;
		float timeSinceLastCheck_;
		float waitInTraffic_checkingInterval_;
	};
}