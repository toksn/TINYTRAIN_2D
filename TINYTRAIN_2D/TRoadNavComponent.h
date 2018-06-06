#pragma once
#include "Component.h"
#include "TLevel.h"
#include "PolyLine.h"

namespace tinytrain
{
	namespace components
	{
		enum class NavType
		{
			RANDOM,			
			SHORTEST_PATH
		};

		class TRoadNavComponent : public tgf::Component
		{
		public:
			TRoadNavComponent(road_network* network);
			~TRoadNavComponent();

			// Inherited via Component
			virtual void draw(sf::RenderTarget * target) override;
			virtual void update(float deltaTime) override;

			float speed_;
			road_network* roads_;
			NavType type_ = NavType::RANDOM;
		protected:
			bool updateNavigation();
			void addEdgeToNavigation(graph::edge* e, bool removePassedWaypoints = false);

			void addNodeConnectionWaypoints(int node_id, direction from, direction to);
			void clearPassedWaypoints();

			// distance travelled on the current waypoints poly
			float distance_;

			// current final edge to travel
			graph::edge* final_edge_;
			tgf::math::PolyLine waypoints_;
			
			//float time_ = 0.0f;
			bool running_;
		};
	}
}