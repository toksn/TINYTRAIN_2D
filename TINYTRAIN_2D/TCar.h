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

		float vmax_;
		components::TRoadNavComponent* navi_;
	private:
		virtual void onUpdate(float deltaTime) override;
	};
}