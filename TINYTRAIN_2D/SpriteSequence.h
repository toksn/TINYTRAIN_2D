#pragma once
#include <SFML/Graphics.hpp>
#include "TextureAtlas.h"

namespace tgf
{
	class SpriteSequence
	{
	public:
		SpriteSequence();
		SpriteSequence(std::string name, tgf::utilities::TextureAtlas* texatlas);
		~SpriteSequence();

		bool getFrame(sf::IntRect& output, int frame_index);
		size_t getFrameCount();
		void addFrame(sf::IntRect rect, int insert_index = -1);

		sf::Texture* getSpriteSheetTexture();

		std::string name_;
	private:
		std::vector<sf::IntRect> frames_;
		sf::Texture* texture_;
	};
}

