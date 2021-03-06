#pragma once
#include <memory>
#include "Entity.h"
#include "TRailTrack.h"
#include "TPassenger.h"
#include "CityGenerator.h"
#include "TRoadNetwork.h"
#include "graph_tgf.h"

#define background_size_factor 2.0f

namespace tgf
{
	namespace utilities
	{
		class TextureAtlas;
	}
}

namespace tinytrain
{
	class TTrain;
	class TObstacle;
	class TCollisionZone;
	class GameState_Running;	

	class TLevel : public tgf::Entity
	{
		friend class TLevel_Builder;

	public:
		struct level_info
		{
			// static info
			std::string introduction_text;
			//sf::Texture introduction_img;
			int car_count;
			int passenger_count;
			int inital_wagon_count;
			// pair<pixel coords (can be float), direction, texcoords>
			std::vector<std::tuple<sf::FloatRect, direction, sf::IntRect>> start_pts;
			std::vector<std::pair<sf::FloatRect, sf::IntRect>> stations;
			std::vector<std::pair<sf::Vector2f, std::string>> deco_images;
			std::string map_file;

			//win/lose conditions
			float timelimit;
			unsigned int points_to_reach;
		};


		TLevel(GameState_Running* gs);
		//TLevel(GameState_Running* gs, const level_info & info);
		~TLevel();

		void restart_();
				
		std::unique_ptr<TTrain> train_;
		std::unique_ptr<TRailTrack> railtrack_;
		std::vector<std::unique_ptr<TObstacle>> obstacles_;
		std::vector<std::unique_ptr<TObstacle>> targetzones_;
		std::vector<std::unique_ptr<TCollisionZone>> static_collision_;
		
		// extra passenger handling needed?
		std::vector<std::unique_ptr<TPassenger>> passengers_;
		std::unique_ptr<TPassenger> removePassenger(unsigned int id);
		void addPassenger(std::unique_ptr<TPassenger> newpass);
		unsigned int passenger_id_;

		sf::Texture arrow_tx_;
		sf::Sprite arrow_;
		float arrow_radius_;
		std::vector<sf::Sprite> arrows_;
		sf::VertexArray inworld_imgs_;

		road_network road_network_;

		// static info
		level_info info_;
		// dynamic info
		unsigned int points_;
		float elapsed_time_;
	protected:
		// Inherited via Entity
		virtual void onDraw(sf::RenderTarget * target) override;
		virtual void onUpdate(float deltaTime) override;

		void onKeyPressed(sf::Event & e);

		void placeArrow(sf::Sprite & a, sf::Vector2f sourcepos, sf::Vector2f targetpos, sf::Color col, float min_alpha_factor = 0.2f);

		sf::VertexArray roads_;
		sf::VertexArray roads_debug_;

		tgf::utilities::TextureAtlas* texture_atlas_ = nullptr;
		float road_texture_width_;
		GameState_Running* gs_;

		sf::VertexArray background_static_;
		sf::VertexArray foreground_static2_;		// drawn in front of background_static, behind foreground_static_
		sf::VertexArray foreground_static_;			// drawn in front of foreground_static2_
		sf::VertexArray foreground_dynamic_;		// drawn in front of both foreground statics
	};
}