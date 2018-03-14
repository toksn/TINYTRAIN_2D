#include "SplineTexture.h"

namespace tgf
{
	namespace utilities
	{
		SplineTexture::SplineTexture()
		{
			spline_ = std::make_unique<Spline_CatmullRom>();
			
			texture_.loadFromFile("data/images/Train/images/track_05.png");
			width_ = texture_.getSize().x;
			//texture_.loadFromFile("data/images/railtrack_marked.png");

			triangles_.setPrimitiveType(sf::PrimitiveType::TrianglesStrip);
			last_processed_startindex_ = -1;
		}

		SplineTexture::~SplineTexture()
		{
		}

		void SplineTexture::onDraw(sf::RenderTarget * target)
		{	
			sf::RenderStates state;
			state.texture = &texture_;
			target->draw(triangles_, state);
			//target->draw(bent_texture);
		}

		void SplineTexture::onUpdate(float deltaTime)
		{
			spline_->update(deltaTime);

			// check if triangles update is necessary
			if (last_processed_startindex_ != spline_->startIndex_lastUpdate_)
			{
				auto size = texture_.getSize();

				sf::Vector2u texture_cuts(1, 2);
				sf::Vector2f texture_seg_size((float)size.x / texture_cuts.x, (float)size.y / texture_cuts.y);
				sf::Vector2f texturePos(0.0f, 0.0f);

				//sf::Color cols[3] = { sf::Color::Red, sf::Color::Green, sf::Color::Blue };

				// start index to redo spline
				int startindex = last_processed_startindex_ + 1;
				if (startindex > spline_->startIndex_lastUpdate_)
					startindex = spline_->startIndex_lastUpdate_;

				triangles_.resize(spline_->splinePoints_.getVertexCount() * 2);
				for (int i = startindex; i < spline_->splinePoints_.getVertexCount(); i++)
				{
					triangles_[i * 2 + 1].position = triangles_[i * 2].position = spline_->splinePoints_[i].position;
					triangles_[i * 2].position += spline_->normals_[i] * width_ / 2.0f;
					triangles_[i * 2 + 1].position -= spline_->normals_[i] * width_ / 2.0f;

					//bent_texture[i * 2 + 1].color = bent_texture[i * 2].color = sf::Color(255, 0, 0, 100);
					//bent_texture[i * 2 + 1].color = cols[(i * 2+1) % 3];
					//bent_texture[i * 2].color = cols[(i*2)%3];
					/*segmente: 4

					0  1  2  3		4  5  6  7			8

					0  8 16 24		32 24 16 8			0

					40 48 56			0
					*/
					int temp = i % (texture_cuts.y * 2);
					int mo = texture_cuts.y + 1;
					if (temp > texture_cuts.y)
					{
						temp = -i;
						mo = texture_cuts.y;
						texturePos.y = size.y;
					}
					else
						texturePos.y = 0.0f;


					texturePos.y += texture_seg_size.y * (temp % (mo));

					triangles_[i * 2 + 1].texCoords = triangles_[i * 2].texCoords = texturePos;
					triangles_[i * 2 + 1].texCoords.x += texture_seg_size.x;
				}
			
				// update last processed startindex
				last_processed_startindex_ = spline_->startIndex_lastUpdate_;
			}
		}
	}
}