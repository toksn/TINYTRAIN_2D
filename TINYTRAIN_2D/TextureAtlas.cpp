#include "TextureAtlas.h"

//#include <string>
//#include <fstream>
//#include <streambuf>
//#include "json_tgf.h"

namespace tgf
{
	namespace utilities
	{
		TextureAtlas::TextureAtlas()
		{
			img_ = std::make_unique<sf::Image>();
		}
		TextureAtlas::~TextureAtlas()
		{
		}

		bool TextureAtlas::init(std::string texture_file_path, std::string jsondata_file_path)
		{
			if (img_ && img_->loadFromFile(texture_file_path))
			{
				texture_coords_.clear();
				tex_ = std::make_unique<sf::Texture>();

				// json data may be empty -> create json path by chaning texture file extension to json
				// todo: read out json data to fill the map
				//texture_coords_[texture_name] = sf::IntRect(x, y, w, h);
				/*std::ifstream t(jsondata_file_path);
				std::string str;

				t.seekg(0, std::ios::end);
				str.reserve(t.tellg());
				t.seekg(0, std::ios::beg);

				str.assign((std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>());
					
				// static functions of a potential json lib
				json::object bla = json::parseFromJsonString(str);
				//std::string somestring = json::parseToJsonString(bla);
				
				auto frames = bla["frames"]->asArray();
				if (frames != nullptr)
				{
					for (auto& data : frames->arraydata)
					{
						if (data->type == json::types::object)
						{
							auto name = data["filename"]->asString();
							auto frame = data["frame"]->asObject();

							if (name && frame)	// && name->type == json::types::string && frame->type == json::types::object)
							{
								std::string key_name = name->string;
								sf::IntRect r;
								auto x = frame["x"]->asInt();
								auto y = frame["y"]->asInt();
								auto w = frame["w"]->asInt();
								auto h = frame["h"]->asInt();
								if (x && y && w && h)
								{
									r.left = x.integer;
									r.top = y.integer;
									r.width = w.integer;
									r.height = h.integer;
								}

								// add frame
								texture_coords_[key_name] = r;
							}
						}
					}
				}*/

				texture_coords_["road"] = sf::IntRect(64, 0, 64, 64);
				texture_coords_["road_crossing-1"] = sf::IntRect(0, 192, 128, 128);
				texture_coords_["road_crossing-2"] = sf::IntRect(0, 64, 128, 128);
				texture_coords_["track_05"] = sf::IntRect(0, 0, 64, 32);

				return true;
			}
	
			return false;
		}

		const sf::Image * TextureAtlas::getImage()
		{
			return img_.get();
		}

		// this function can be used to receive a pointer to a texture of the whole textureatlas image. 
		// the texture memory is managed by the textureAtlas and the pointer may change when invoking init more than once
		sf::Texture * TextureAtlas::getTexture()
		{
			if (img_ && tex_)
			{
				auto size = tex_->getSize();
				if (size.x == 0 || size.y == 0)
					tex_->loadFromImage(*img_);
			}

			return tex_.get();
		}

		sf::IntRect TextureAtlas::getArea(std::string texture_name)
		{
			if (texture_coords_.count(texture_name))
				return texture_coords_[texture_name];

			return sf::IntRect();
		}
	}
}
