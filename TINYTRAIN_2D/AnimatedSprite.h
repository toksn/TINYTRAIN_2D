#pragma once
#include "SpriteSequence.h"
#include "tgfdefines.h"

namespace tgf
{
	class AnimatedSprite : public sf::Transformable, public sf::Drawable
	{
	public:
		AnimatedSprite();
		AnimatedSprite(SpriteSequence* seq);
		~AnimatedSprite();

		// Inherited via Drawable
		virtual void draw(sf::RenderTarget & target, sf::RenderStates states) const override;
		
		void update(float time);
		void setFrameSequence(SpriteSequence* seq);
		void setDirectionMode(directionMode dir, bool repeat = true);
		void setFPS(unsigned int fps);
		void setColor(sf::Color col);
		void setTexture(sf::Texture* tex);
		void setRandomFrame();

		const sf::Vector2f& getCurrentFrameSize();

		void run();
		void pause();
		void reset();
		//void start();	//reset+run
		//void stop();	//pause+reset

		float frametime_;
	private:
		void nextFrame();
		void placeCurrentFrame();

		SpriteSequence* frameSequence_;
		sf::Texture* texture_;

		unsigned int frame_;

		bool repeat_;
		directionMode dir_;

		float time_;
		bool running_;
		
		sf::Vertex vertices_[4];
	};
}