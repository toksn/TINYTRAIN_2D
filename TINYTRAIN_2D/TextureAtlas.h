#pragma once
#include <memory>
#include <SFML\Graphics.hpp>
//#include <map>

namespace tgf
{
	namespace utilities
	{
		class TextureAtlas
		{
		public:
			TextureAtlas();
			~TextureAtlas();

			bool init(std::string texture_file_path, std::string jsondata_file_path = "");

			const sf::Image* getImage();
			sf::IntRect getArea(std::string texture_name);
		private:
			std::unique_ptr<sf::Image> img_;
			std::map<std::string, sf::IntRect> texture_coords_;
		};
	}
}