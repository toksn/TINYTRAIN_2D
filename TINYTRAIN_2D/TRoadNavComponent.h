#pragma once
#include "Component.h"
#include "TLevel.h"
#include "CollisionManager.h"
#include "PolyLine.h"

namespace tinytrain
{
	namespace components
	{
		
		class TRoadNavComponent : public tgf::Component, public ICrossingUser
		{
		public:
			struct node_travel_info
			{
				int node_id_;
				float distEnd_;		// used as start and end dist for the crossing
				float distStart_;	// used as start dist for the next crossing
				direction from; 
				direction to;
				bool started = false;
			};

			enum class NavType
			{
				RANDOM,
				SHORTEST_PATH
			};
			enum class NavState
			{
				RUNNING_,
				RUNNING_ON_CROSSING,
				RUNNING_WAIT_FOR_CLEAR_ROAD,
				STOPPED_
			};
		
			TRoadNavComponent(road_network* network, tgf::collision::CollisionManager* collision);
			~TRoadNavComponent();

			// Inherited via Component
			virtual void draw(sf::RenderTarget * target) override;
			virtual void update(float deltaTime) override;
			NavState getState();

			float speed_;
			road_network* roads_;
			NavType type_ = NavType::RANDOM;
			unsigned int roadCheckingMask = tgf::collision::CollisionManager::CollisionCategory::DYNAMIC_CATEGORY_1;
			bool debugDraw_;

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

			// current node travelling
			std::unique_ptr<node_travel_info> cur_node_;

			// stopping points and collision manager
			std::unique_ptr<road_connection_info::stopping_info> stopper_;
			tgf::collision::CollisionManager* collision_;
			sf::VertexArray debugCrossingWaypoints_;

			NavState state_;
			
			//float time_ = 0.0f;
			bool running_;

			// Inherited via ICrossingUser
			virtual void startCrossing(int node_id, direction from, direction to) override;
		};
	}
}