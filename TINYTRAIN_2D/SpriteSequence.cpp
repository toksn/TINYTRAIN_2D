#include "SpriteSequence.h"



namespace tgf
{
	SpriteSequence::SpriteSequence()
	{
	}

	// call this with an animation name to be automatically read out from the atlas
	// it is searched for the following pattern to find sprites that are filled 
	// into the frames of this sequence:
	// "ani_" + name + "_" 
	// after that prefix, the alphabetical order is used to order the frames.
	// f.e. one can use the format ani_somename_1, ani_somename_2, ... in the atlas
	// to initialize the spritesequence "somename"
	SpriteSequence::SpriteSequence(std::string name, tgf::utilities::TextureAtlas * atlas)
	{
		name_ = name;

		if (atlas)
		{
			texture_ = atlas->getTexture();

			// fill frames from ani_name_xx
			const std::string prefix = "ani_" + name + "_";
			auto first = atlas->texture_coords_.lower_bound(prefix);
			auto second = atlas->texture_coords_.upper_bound(prefix + "zzz");
			if (first != atlas->texture_coords_.end() && second != atlas->texture_coords_.end())
			{
				if(first == second)
					printf("SpriteSequence ctor: could not find any ani_%s_[] entries in the texture atlas\n", name.c_str());

				for (auto it = first; it != second; ++it)
				{
					std::string suffix = it->first.substr(prefix.length());
					if (suffix.length() > 0)
					{
						// this doesnt have to be numbered but the alphabetical order of the frames is preserved in the frames_ vector
						frames_.push_back(it->second);
					}
				}
			}
			else
				printf("SpriteSequence ctor: could not find any ani_%s_[] entries in the texture atlas\n", name.c_str());
		}
	}

	SpriteSequence::~SpriteSequence()
	{
	}

	bool SpriteSequence::getFrame(sf::IntRect& output, int frame_index)
	{
		if (frames_.size() == 0 || frame_index < 0 || frame_index > frames_.size() - 1)
			return false;

		output = frames_[frame_index];
		return true;
	}

	size_t SpriteSequence::getFrameCount()
	{
		return frames_.size();
	}

	void SpriteSequence::addFrame(sf::IntRect rect, int insert_index)
	{
		if (insert_index < 0 || insert_index >= frames_.size() - 1)
			frames_.push_back(rect);
		else
			frames_.insert(frames_.begin() + insert_index, rect);
	}

	sf::Texture * SpriteSequence::getSpriteSheetTexture()
	{
		return texture_;
	}
}
