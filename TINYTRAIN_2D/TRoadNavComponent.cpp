#include "TRoadNavComponent.h"

namespace tinytrain
{
	namespace components
	{
		TRoadNavComponent::TRoadNavComponent(road_network * network, tgf::collision::CollisionManager * collision)
		{
			roads_ = network;
			collision_ = collision;
			speed_ = 1.0f;
			running_ = true;
			drawDebug_ = false;

			debugCrossingWaypoints_.setPrimitiveType(sf::PrimitiveType::LinesStrip);

			respawn(false);
		}
		TRoadNavComponent::~TRoadNavComponent()
		{
		}

		void TRoadNavComponent::draw(sf::RenderTarget * target)
		{
			if (drawDebug_)
			{
#if USE_STOPPING_POINTS
				if (stopper_)
				{
					for (auto& a : stopper_->areas_to_check_before_continue)
					{
						sf::VertexArray v;
						v.setPrimitiveType(sf::PrimitiveType::LineStrip);
						v.resize(5);
						v[0].position.x = v[3].position.x = v[4].position.x = a.left;
						v[1].position.x = v[2].position.x = a.left + a.width;

						v[0].position.y = v[1].position.y = v[4].position.y = a.top;
						v[2].position.y = v[3].position.y = a.top + a.height;
						v[0].color = v[1].color = v[2].color = v[3].color = v[4].color = sf::Color::Cyan;
						target->draw(v);
					}
				}
#else
				if(state_ == NavState::RUNNING_WAIT_FOR_CLEAR_ROAD)
					target->draw(debugCrossingWaypoints_);
#endif
			}
		}

		void TRoadNavComponent::update(float deltaTime)
		{
			bool getinfo = true;
			
			if (state_ != NavState::STOPPED_ && speed_ > 0.0f)
			{
#if USE_STOPPING_POINTS
				if (state_ == NavState::RUNNING_WAIT_FOR_CLEAR_ROAD)
				{
					if (stopper_ && collision_)
					{
						bool roads_clear = true;
						// check all stopper_->areas_to_check_before_continue to be empty
						for (auto& rect : stopper_->areas_to_check_before_continue)
						{
							c2AABB r;
							r.min = { rect.left, rect.top };
							r.max = { rect.left + rect.width, rect.top + rect.height };
							tgf::collision::c2Shape s;
							s.shape_ = &r;
							s.type_ = C2_TYPE::C2_AABB;
							unsigned int mask = 0;
							mask |= tgf::collision::CollisionManager::CollisionCategory::DYNAMIC_CATEGORY_1;
							if (collision_->checkShapeForCollisions(s, mask))
							{
								roads_clear = false;
								break;
							}
						}

						if (roads_clear)
						{
							state_ = NavState::RUNNING_;
							stopper_.reset(nullptr);
						}
							
					}
					else
					{
						state_ = NavState::RUNNING_;
						stopper_.reset(nullptr);
					}
				}
#endif				
				if( state_ != NavState::RUNNING_WAIT_FOR_CLEAR_ROAD )
				//if (state_ == NavState::RUNNING_ || state_ == NavState::RUNNING_ON_CROSSING)
				{
					distance_ += deltaTime*speed_;

#if USE_STOPPING_POINTS
					// change for stopping points
					if (stopper_ && distance_ >= stopper_->stop_at_dist)
					{
						distance_ = stopper_->stop_at_dist;
						if (distance_ - stopper_->stop_at_dist < 12.0f)
						{
							state_ = NavState::RUNNING_WAIT_FOR_CLEAR_ROAD;
							//return;
						}
					}
#else
					if (cur_node_.valid)
					{
						if (state_ == NavState::RUNNING_ON_CROSSING)
						{
							if (distance_ >= cur_node_.distEnd_)
							{
								state_ = NavState::RUNNING_;
								roads_->removeFromCrossing(this, cur_node_.node_id_);
								cur_node_.valid = false;
							}
							else if (cur_node_.distEnd_ > 0.0f)
								roads_->updateCrossingProgression(this, cur_node_.node_id_, distance_ / cur_node_.distEnd_);
						}
						else if (state_ == NavState::RUNNING_)
						{
							if (cur_node_.started == false && distance_ >= cur_node_.distStart_)
							{
								cur_node_.started = true;
								if (roads_->road_graph.nodes_[cur_node_.node_id_].edges_.size() > 2)
								{
									distance_ = cur_node_.distStart_;
									state_ = NavState::RUNNING_WAIT_FOR_CLEAR_ROAD;
									roads_->applyToCrossing(this, cur_node_.node_id_, cur_node_.from, cur_node_.to);
								}
								else
									cur_node_.valid = false;
							}
						}
					}
#endif

					// max dist to travel on the current waypoints
					float maxdist = waypoints_.getLength();

					if (distance_ > maxdist)
					{
						getinfo = updateNavigation();
					}
					else if (distance_ < 0.0f)
					{
						// todo: event of invalid distance
						distance_ = 0.0f;
					}

					if (getinfo)
					{
						float time = distance_ / maxdist;

						int hintindex = -1;
						float angle = waypoints_.getDirectionAngleAtTime(time, hintindex, false);
						sf::Vector2f pos = waypoints_.getLocationAtTime(time, hintindex);
						owner_->setPosition(pos);
						owner_->setRotation(angle);
					}
				}
			}
		}

		std::unique_ptr<tgf::Component> TRoadNavComponent::cloneComponent()
		{
			// normal (shallow copy) is enough to copy this component
			return std::make_unique<TRoadNavComponent>(*this);
		}

		TRoadNavComponent::NavState TRoadNavComponent::getState()
		{
			return state_;
		}

		bool TRoadNavComponent::respawn(bool onlyDeadends)
		{
			if (roads_ == nullptr)
				return false;


			// TODO: only use deadends? or check for existing car at that position, move some distance
			if (onlyDeadends)
			{
				if (roads_->deadends.size())
				{
					int deadend_node_id = roads_->deadends[rand() % roads_->deadends.size()];
					auto& n = roads_->road_graph.nodes_[deadend_node_id];

					if (n.edges_.size() == 1)
					{
						auto& e = n.edges_.front();

						// adding edge as the only edge at the moment (first edge)
						addEdgeToNavigation(&e, true);
						return true;
					}
				}
			}
			// respawn: randomly choose one to start from
			else if (roads_ != nullptr && roads_->road_graph.nodes_.size())
			{
				auto n = roads_->road_graph.nodes_.begin();
				std::advance(n, rand() % roads_->road_graph.nodes_.size());
				if (n->second.edges_.size())
				{
					auto e = n->second.edges_.begin();
					std::advance(e, rand() % n->second.edges_.size());

					// adding edge as the only edge at the moment (first edge)
					addEdgeToNavigation(&*e, true);
					return true;
				}
			}

			return false;
		}

		bool TRoadNavComponent::updateNavigation()
		{
			bool rc = false;
			if (final_edge_ == nullptr)
			{
				rc = respawn(true);
			}
			else
			{
				auto& edges = roads_->road_graph.nodes_[final_edge_->target_node_].edges_;
				
				// deadend 
				if (edges.size() <= 1)
				{
					clearPassedWaypoints();
					addNodeConnectionWaypoints(final_edge_->target_node_, final_edge_->user_data_.in_slot, (direction)((final_edge_->user_data_.in_slot+2)%direction::DIR_COUNT));
					// respawn when current waypoints are done
					final_edge_ = nullptr;

					waypoints_.recalcLength();
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
					} while (e->user_data_.out_slot == final_edge_->user_data_.in_slot);

					addEdgeToNavigation(&*e, true);
					rc = true;
				}
			}
			return rc;
		}

		void TRoadNavComponent::addEdgeToNavigation(graph::edge * e, bool removePassedWaypoints)
		{
			if (removePassedWaypoints)
			{
#if 0
				// just clear it already
				distance_ -= waypoints_.getLength();
				waypoints_.poly_.clear();
#else			
				clearPassedWaypoints();
#endif // 1
			}

			if (e != nullptr)
			{
				if (roads_ != nullptr)
				{
					if (final_edge_ != nullptr)
					{
						// add waypoints of the crossing from in (old edge) to out (new edge)
						//
						//			^
						//			| new edge
						//			|
						//		  (out)
						//		[CROSSING](in)<-----final (old) edge

						direction from = final_edge_->user_data_.in_slot;
						direction to = e->user_data_.out_slot;
						addNodeConnectionWaypoints(final_edge_->target_node_, from, to);
					}
					else
					{
						// no existing edge, this is a new entry into the graph. make sure to use the crossing waypoints as well
						if (roads_->road_graph.nodes_[e->source_node_].edges_.size() == 1)
							addNodeConnectionWaypoints(e->source_node_, (direction)((e->user_data_.out_slot + 2) % direction::DIR_COUNT), e->user_data_.out_slot);
						else
						{
							for (auto& fromedge : roads_->road_graph.nodes_[e->source_node_].edges_)
							{
								if (&fromedge != e)
								{
									addNodeConnectionWaypoints(e->source_node_, fromedge.user_data_.out_slot, e->user_data_.out_slot);
									break;
								}
							}
						}
					}
				}


				// set new final edge
				final_edge_ = e;

				// add waypoints of the new edge
				waypoints_.poly_.insert(waypoints_.poly_.end(), final_edge_->user_data_.waypoints.begin(), final_edge_->user_data_.waypoints.end());
			}

			waypoints_.recalcLength();
		}

		void TRoadNavComponent::addNodeConnectionWaypoints(int node_id, direction from, direction to)
		{
			auto& node = roads_->road_graph.nodes_[node_id];
			sf::Vector2f curpos(node.user_data_.left, node.user_data_.top);

			//sf::Vector2f curpos = final_edge_->user_data_.waypoints.back();
			//curpos.x -= roads_->crossing_connection_table[from][to].waypoints.front().x;
			//curpos.y -= roads_->crossing_connection_table[from][to].waypoints.front().y;

			if(roads_->road_graph.nodes_[node_id].edges_.size() > 2)
			{
				if (cur_node_.valid)
				{
					printf("TRoadNav error: trying to override node_travel_info without completing the current one!\n");
				}
				else
				{
					cur_node_.valid = true;
					cur_node_.node_id_ = node_id;
					cur_node_.distStart_ = waypoints_.getLength();
					cur_node_.distEnd_ = 0.0f;
					cur_node_.started = false;
					cur_node_.from = from;
					cur_node_.to = to;
				}
			}

			for (auto& pt : roads_->crossing_connection_table[from][to].waypoints)
				waypoints_.poly_.emplace_back(pt.x + curpos.x, pt.y + curpos.y);

#if USE_STOPPING_POINTS
			// when the crossing waypoints (from;to) has areas to check, save that info				
			if (node.edges_.size() > 2)
			{
				bool rotate = false;
				if (node.edges_.size() == 3)
				{
					int northSouth_dir = 0;
					for (auto& e : node.edges_)
						if (e.user_data_.out_slot % 2 == 0)
							northSouth_dir++;
					rotate = northSouth_dir < 2;
				}

				if (rotate)
				{
					if (roads_->crossing_connection_table[from][to].rotatedstopinfo.areas_to_check_before_continue.size() > 0)
						stopper_ = std::make_unique<road_connection_info::stopping_info>(roads_->crossing_connection_table[from][to].rotatedstopinfo);
				}
				else
				{
					if (roads_->crossing_connection_table[from][to].stopinfo.areas_to_check_before_continue.size() > 0)
						stopper_ = std::make_unique<road_connection_info::stopping_info>(roads_->crossing_connection_table[from][to].stopinfo);
				}
					

				if (stopper_)
				{
					for (auto& rect : stopper_->areas_to_check_before_continue)
					{
						rect.top += curpos.y;
						rect.left += curpos.x;
					}
				}
			}
#else
			debugCrossingWaypoints_.resize(roads_->crossing_connection_table[from][to].waypoints.size());
			int i = 0;
			for (auto& pt : roads_->crossing_connection_table[from][to].waypoints)
				debugCrossingWaypoints_[i++].position = { pt.x + curpos.x, pt.y + curpos.y };
#endif
			
		}

		void TRoadNavComponent::clearPassedWaypoints()
		{
			if (distance_ >= waypoints_.getLength())
			{
				distance_ -= waypoints_.getLength();
				waypoints_.poly_.clear();
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
		}

		void TRoadNavComponent::startCrossing(int node_id, direction from, direction to)
		{
			state_ = NavState::RUNNING_ON_CROSSING;
			if(cur_node_.valid)
				cur_node_.distEnd_ = roads_->crossing_connection_table[from][to].distance;
		}
	}
}