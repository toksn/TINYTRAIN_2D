#include "TRoadNavComponent.h"

namespace tinytrain
{
	namespace components
	{
		TRoadNavComponent::TRoadNavComponent(road_network * network)
		{
		}
		TRoadNavComponent::~TRoadNavComponent()
		{
		}

		void TRoadNavComponent::draw(sf::RenderTarget * target)
		{
		}

		void TRoadNavComponent::update(float deltaTime)
		{
		}

		bool TRoadNavComponent::updateNavigation()
		{
			bool rc = false;
			if (final_edge_ == nullptr)
			{
				// randomly choose one to start from?
				if (roads_ != nullptr && roads_->road_graph.nodes_.size())
				{
					auto n = roads_->road_graph.nodes_.begin();
					std::advance(n, rand() % roads_->road_graph.nodes_.size());
					if (n->second.edges_.size())
					{
						auto e = n->second.edges_.begin();
						std::advance(e, rand() % n->second.edges_.size());
						
						// adding edge as the only edge at the moment (first edge)
						rc = addEdge(&*e, true);
					}
				}					
			}
			else
			{
				auto& edges = roads_->road_graph.nodes_[final_edge_->target_node_].edges_;
				
				// deadend 
				if (edges.size() <= 1)
				{
					// respawn. or better stop?
					final_edge_ = nullptr;
					rc = true;
				}
				// advance current final edge to something else
				else if (type_ == NavType::RANDOM)
				{
					auto e = edges.begin();
					// find first edge that doesnt go directly back
					do
					{
						e = edges.begin();
						std::advance(e, rand() % edges.size());
					} while (e->user_data_.out_slot != final_edge_->user_data_.in_slot);

					rc = addEdge(&*e, true);
				}
			}

			return rc;
		}
		bool TRoadNavComponent::addEdge(graph::edge * e, bool removePassedWaypoints)
		{
			bool rc = false;
			
			if (removePassedWaypoints)
			{
#if 0
				// just clear it already
				waypoints_.poly_.clear();
				distance_ = 0.0f;
#else			
				if (distance_ >= waypoints_.getLength())
				{
					waypoints_.poly_.clear();
					distance_ = 0.0f;
				}
				else
				{
					int remove_index = waypoints_.getSegmentStartIndexAtDist(distance_);
					float len = waypoints_.lengths_[remove_index];
					if (remove_index >= 0)
					{
						waypoints_.poly_.erase(waypoints_.poly_.begin(), waypoints_.poly_.begin() + remove_index);
						distance_ = distance_ - len;
					}
				}
#endif // 1
				waypoints_.recalcLength();
			}

			if (e != nullptr)
			{
				if (final_edge_ != nullptr && roads_ != nullptr)
				{
					// add waypoints of the crossing from in (old edge) to out (new edge)
					//
					//			^
					//			| new edge
					//			|
					//		  (out)
					//		[CROSSING](in)<-----final (old) edge
					//
					direction from = final_edge_->user_data_.in_slot;
					direction to = e->user_data_.out_slot;

					//auto& crossing_wp = roads_->crossing_connection_table[from][to].waypoints;
					//waypoints_.poly_.emplace_back(crossing_wp.begin(), crossing_wp.end());
					
					sf::Vector2f curpos = final_edge_->user_data_.waypoints.back();
					curpos.x -= roads_->crossing_connection_table[from][to].waypoints.front().x;
					curpos.y -= roads_->crossing_connection_table[from][to].waypoints.front().y;

					for (auto& pt : roads_->crossing_connection_table[from][to].waypoints)
						waypoints_.poly_.emplace_back(pt.x + curpos.x, pt.y + curpos.y);
				}
				
				// add waypoints of the new edge
				waypoints_.poly_.insert(waypoints_.poly_.end(), final_edge_->user_data_.waypoints.begin(), final_edge_->user_data_.waypoints.end());

				// set new final edge
				final_edge_ = e;
			}
			return rc;
		}
	}
}