#include "TextureAtlas.h"

#include <string>
#include <fstream>
#include <streambuf>
#include "json_tgf.h"

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

				if (jsondata_file_path.length() == 0)
				{
					jsondata_file_path = texture_file_path;
					std::size_t ext_pos = jsondata_file_path.rfind('.');
					if (ext_pos != std::string::npos)
					{
						jsondata_file_path.resize(ext_pos + 1);
						jsondata_file_path.append("json");
					}
				}
					

				// json data may be empty -> create json path by chaning texture file extension to json
				// todo: read out json data to fill the map
				//texture_coords_[texture_name] = sf::IntRect(x, y, w, h);
				std::ifstream t(jsondata_file_path);
				std::string str;

				t.seekg(0, std::ios::end);
				str.reserve(t.tellg());
				t.seekg(0, std::ios::beg);

				str.assign((std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>());
					
				// static functions of a potential json lib
				json::object bla = json::parseFromJsonString(str);
				//std::string somestring = json::parseToJsonString(bla);
				
				auto frames = bla["frames"].as_array();
				if (frames != nullptr)
				{
					for (auto it = frames->begin(); it != frames->end(); ++it)
					{
						auto data = it->as_object();
						if( data != nullptr )
						//if (data.type == json::type::OBJECT)
						{
							auto name = (*data)["filename"].as_string();
							auto frame = (*data)["frame"].as_object();

							if (name && frame)	// && name->type == json::types::string && frame->type == json::types::object)
							{
								//std::string key_name = name->string;
								sf::IntRect r;
								auto x = (*frame)["x"].as_int();
								auto y = (*frame)["y"].as_int();
								auto w = (*frame)["w"].as_int();
								auto h = (*frame)["h"].as_int();
								if (x && y && w && h)
								{
									r.left = *x;
									r.top = *y;
									r.width = *w;
									r.height = *h;
								}

								// add frame
								texture_coords_[*name] = r;
							}
						}
					}
				}

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
