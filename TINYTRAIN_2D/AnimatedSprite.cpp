#include "AnimatedSprite.h"



namespace tgf
{
	AnimatedSprite::AnimatedSprite()
	{
		frameSequence_ = nullptr;
		texture_ = nullptr;
		running_ = false;
		time_ = 0.0f;
		frametime_ = 1.0f / 10.0f; // 10fps animation
		frame_ = 0;
	}

	AnimatedSprite::AnimatedSprite(SpriteSequence * seq)
	{
		frameSequence_ = seq;
		if (seq)
			texture_ = seq->getSpriteSheetTexture();

		running_ = false;
		time_ = 0.0f;
		frametime_ = 1.0f / 10.0f; // 10fps animation
		frame_ = 0;

		// init frame 0
		placeCurrentFrame();
	}


	AnimatedSprite::~AnimatedSprite()
	{
	}

	void AnimatedSprite::draw(sf::RenderTarget & target, sf::RenderStates states) const
	{
		// draw the current intrect of the texture
		if (texture_ && frameSequence_)
		{
			states.transform *= getTransform();
			states.texture = texture_;
			target.draw(vertices_, 4, sf::PrimitiveType::Quads, states);
		}
	}
	void AnimatedSprite::update(float time)
	{
		// calc if next frame is needed
		if (running_)
		{
			time_ += time;
			
			// put on next frame
			if (time_ >= frametime_)
			{
				nextFrame();
				time_ -= frametime_;
			}
		}
	}

	void AnimatedSprite::setFrameSequence(SpriteSequence * seq)
	{
		frameSequence_ = seq;
		if (seq)
			texture_ = seq->getSpriteSheetTexture();

		frame_ = 0;
		time_ = 0.0f;
		placeCurrentFrame();
	}

	void AnimatedSprite::setDirectionMode(directionMode dir, bool repeat)
	{
		dir_ = dir;
		repeat_ = repeat;
	}

	void AnimatedSprite::setFPS(unsigned int fps)
	{
		frametime_ = 1.0f / fps;
	}

	void AnimatedSprite::setColor(sf::Color col)
	{
		for (int i = 0; i < 4; i++)
			vertices_[i].color = col;
	}

	void AnimatedSprite::setTexture(sf::Texture * tex)
	{
		texture_ = tex;
	}

	const sf::Vector2f & AnimatedSprite::getCurrentFrameSize()
	{
		return vertices_[2].position;
	}

	void AnimatedSprite::run()
	{
		running_ = true;
	}
	void AnimatedSprite::pause()
	{
		running_ = false;
	}
	void AnimatedSprite::reset()
	{
		time_ = 0.0f;
	}

	void AnimatedSprite::nextFrame()
	{
		frame_++;

		// check for the end, wrap around if repeat is on
		if ((dir_ == directionMode::OneWay && frame_ >= frameSequence_->getFrameCount()) || 
			(dir_ == directionMode::TwoWay && frame_+2 >= frameSequence_->getFrameCount() * 2))
		{
			if (repeat_)
				frame_ = 0;
			else
				pause();
		}

		if (running_)
			placeCurrentFrame();
	}
	void AnimatedSprite::placeCurrentFrame()
	{
		int actual_frame = frame_;
		//0>1>2>3>4
		//0>1>2>1>0
		if (dir_ == directionMode::TwoWay && frame_ >= frameSequence_->getFrameCount())
			actual_frame = 2 * frameSequence_->getFrameCount() - frame_ - 2;

		sf::IntRect rect;
		if (frameSequence_->getFrame(rect, actual_frame))
		{
			vertices_[0].position.x = 0.0f;
			vertices_[0].position.y = 0.0f;
			vertices_[1].position.x = rect.width;
			vertices_[1].position.y = 0.0f;
			vertices_[2].position.x = rect.width;
			vertices_[2].position.y = rect.height;
			vertices_[3].position.x = 0.0f;
			vertices_[3].position.y = rect.height;

			vertices_[0].texCoords.x = rect.left;
			vertices_[0].texCoords.y = rect.top;

			vertices_[1].texCoords = vertices_[2].texCoords = vertices_[3].texCoords = vertices_[0].texCoords;

			vertices_[1].texCoords.x += rect.width;


			vertices_[2].texCoords.x += rect.width;
			vertices_[2].texCoords.y += rect.height;

			vertices_[3].texCoords.y += rect.height;
		}
	}
}