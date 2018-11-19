#include "TPassenger.h"
#include "GameState_Running.h"
#include "TRoadNavComponent.h"
#include "TLevel.h"

namespace tinytrain
{
	TPassenger::TPassenger(GameState_Running * gs) : TCollisionZone(gs, true)
	{
		sf::RectangleShape rect(sf::Vector2f(20.0f, 20.0f));
		destination_drawable_ = std::make_unique<sf::RectangleShape>(rect);

		rect.setFillColor(winningTrigger_ ? sf::Color::Green : sf::Color::Red);
		drawable_ = std::make_unique<sf::RectangleShape>(rect);

		collision_quad_.setPrimitiveType(sf::PrimitiveType::LineStrip);
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Yellow));
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Yellow));
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Yellow));
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Yellow));
		collision_quad_.append(sf::Vertex(sf::Vector2f(), sf::Color::Yellow));

		if (gs && gs->game_)
		{
			spriteSeq_ = std::make_unique<tgf::SpriteSequence>("passenger", gs->game_->getTextureAtlas());
			sprite_ = std::make_unique<tgf::AnimatedSprite>(spriteSeq_.get());
			//sprite_->setTexture(spriteSeq_->getSpriteSheetTexture());
			sprite_->setFrameSequence(spriteSeq_.get());

			sprite_->setRandomFrame();
			sprite_->setDirectionMode(directionMode::TwoWay, true);
			sprite_->run();
		}
		else
		{
			spriteSeq_ = nullptr;
			sprite_ = nullptr;
		}			

		points_ = 1;
		state_ = PassengerState::WAIT_FOR_PICKUP;
	}
	
	TPassenger::~TPassenger()
	{
	}

	void TPassenger::reset()
	{
		setState(PassengerState::WAIT_FOR_PICKUP);
	}

	void TPassenger::setState(TPassenger::PassengerState newstate)
	{
		if (state_ != newstate)
		{
			drawable_.swap(destination_drawable_);

			updateCollisionShape_matchDrawable();

			state_ = newstate;
		}

		//if (newstate == PassengerState::COLLECTED)
		//{
		//	
		//}
	}

	TPassenger::PassengerState TPassenger::getState()
	{
		return state_;
	}

	void TPassenger::updateCollisionShape_matchDrawable()
	{
		TCollisionZone::updateCollisionShape_matchDrawable();

		if (sprite_)
		{
			auto sprite_size = sprite_->getCurrentFrameSize();
			auto rect_size = drawable_->getSize();

			// scale the sprite in relation to the rect_size (fe. 5/8 of 64px are 40px)
			float scale_x = ((5.0f/8.0f) *rect_size.x) / sprite_size.x;
			float scale_y = ((5.0f/8.0f) *rect_size.y) / sprite_size.y;
			sprite_->setScale(scale_x, scale_y);
			sprite_size.x *= scale_x;
			sprite_size.y *= scale_y;
			
			sf::Vector2f pos;
			pos.x = rect_size.x * 0.5f - sprite_size.x * 0.5f;
			pos.y = sprite_size.y * -0.2f;

			pos += drawable_->getPosition();
			sprite_->setPosition(pos);
		}
	}

	void TPassenger::onDraw(sf::RenderTarget * target)
	{
		TCollisionZone::onDraw(target);

		if(drawable_)
			target->draw(*drawable_);

		if (sprite_)
			target->draw(*sprite_);

		if (drawDebug_)
			target->draw(collision_quad_);
	}

	void TPassenger::onUpdate(float deltaTime)
	{
		TCollisionZone::onUpdate(deltaTime);

		if(sprite_)
			sprite_->update(deltaTime);
	}


	void TPassenger::onTriggerEnter(tgf::collision::CollisionEntity * other)
	{
	}
}
