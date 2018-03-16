#include "SplineTexture.h"
#include "tinyc2.h"

namespace tgf
{
	namespace utilities
	{
		SplineTexture::SplineTexture()
		{
			spline_ = std::make_unique<Spline_CatmullRom>();
			
			// todo: make texture available from outside
			texture_.loadFromFile("data/images/track/track_05.png");
			//texture_.loadFromFile("data/images/track/railtrack_marked.png");
			width_ = texture_.getSize().x;
			

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
				
				// start index to redo spline
				int startindex = last_processed_startindex_ + 1;
				if (startindex > spline_->startIndex_lastUpdate_)
					startindex = spline_->startIndex_lastUpdate_;

				// create new triangles
				createTriangleStripFromSpline(startindex);
			
				// update last processed startindex
				last_processed_startindex_ = spline_->startIndex_lastUpdate_;
			}
		}

		void SplineTexture::createTriangleStripFromSpline(int startindex)
		{
			auto size = texture_.getSize();
			float tex_scale = width_ / size.x;
			float rest_len = 0.0f;
			int tri_index = startindex;
			//size.x = width_;

			bool useSplineptsForTextureSplitting = false;				
			//sf::Color cols[3] = { sf::Color::Red, sf::Color::Green, sf::Color::Blue };
			
			// **********************************************************
			// SETTINGS
			// **********************************************************
			c2v lastSplinePt = { spline_->splinePoints_[startindex].position.x, spline_->splinePoints_[startindex].position.y };
			sf::Vector2f texturePos(0.0f, 0.0f);
			
			bool forward = true;
			if (startindex > 0)
			{
				
				//sf::Vertex lastVert2 = triangles_[(startindex - 1) * 2 + 1];

				lastSplinePt = { spline_->splinePoints_[startindex-1].position.x, spline_->splinePoints_[startindex-1].position.y };

				// find tri_index to start from
				float startlen = spline_->splinePointsLengths_[startindex-1];
				for (; tri_index*2 < trianglesLengths_.size(); tri_index++)
				{
					//if(trianglesLengths_[tri_index * 2] == startlen)


					if (trianglesLengths_[tri_index * 2] > startlen)
					{
						break;
					}
						
				}
				tri_index--;
				sf::Vertex lastVert = triangles_[(tri_index)*2];
				texturePos.y = lastVert.texCoords.y;
				if (lastVert.texCoords.y == 0.0f)
				{
					forward = true;
					rest_len = startlen - trianglesLengths_[(tri_index )* 2];
				}
				else if (lastVert.texCoords.y == size.y)
				{
					forward = false;
					rest_len = startlen - trianglesLengths_[(tri_index) * 2];
				}
				else if (startindex > 1)
				{
					forward = lastVert.texCoords.y > triangles_[(tri_index - 1) * 2].texCoords.y;

					rest_len = startlen - trianglesLengths_[(tri_index) * 2];
					if (rest_len != 0.0f)
					{
						printf("todo: rest_len calc is probably wrong. rest: %f\n", rest_len);
					}
					//if (forward)
					//	printf("adding texture going forward!\n");
					//else
					//	printf("adding texture going backward!\n");
				}

				tri_index++;
			}
				

			if (useSplineptsForTextureSplitting)
			{
				triangles_.resize(spline_->splinePoints_.getVertexCount() * 2);
				trianglesLengths_.resize(spline_->splinePoints_.getVertexCount() * 2);
			}
			else
			{
				if (startindex != tri_index)
					printf("tri_index = %i, start_index = %i\n", tri_index, startindex);
				//float seglen = spline_->getLength() / spline_->splinePoints_.getVertexCount();
				triangles_.resize((spline_->splinePoints_.getVertexCount() + tri_index - startindex)* 2);
				trianglesLengths_.resize((spline_->splinePoints_.getVertexCount() + tri_index - startindex) * 2);
			}

			// **********************************************************
			// **********************************************************
			// using splinepoints for uniform texture splitting
			sf::Vector2u texture_cuts(1, 2);
			sf::Vector2f texture_seg_size((float)size.x / texture_cuts.x, (float)size.y / texture_cuts.y);
			// **********************************************************
			// using spline segment length for texture splitting
			
			// **********************************************************
			
			for (int i = startindex; i < spline_->splinePoints_.getVertexCount(); i++)
			{
				if (useSplineptsForTextureSplitting)
				{
					// create triangle strip for the spline
					triangles_[i * 2 + 1].position = spline_->splinePoints_[i].position;
					triangles_[i * 2].position = spline_->splinePoints_[i].position;

					triangles_[i * 2].position += spline_->normals_[i] * width_ / 2.0f;
					triangles_[i * 2 + 1].position -= spline_->normals_[i] * width_ / 2.0f;

					//bent_texture[i * 2 + 1].color = bent_texture[i * 2].color = sf::Color(255, 0, 0, 100);
					//bent_texture[i * 2 + 1].color = cols[(i * 2+1) % 3];
					//bent_texture[i * 2].color = cols[(i*2)%3];

					/*cuts: 4

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
				else // use spline segment len for texturesplitting
				{
					// measure current spline segment and add existing rest_len
					c2v curSplinePt = { spline_->splinePoints_[i].position.x, spline_->splinePoints_[i].position.y };
					float len = rest_len + c2Len(c2Sub(curSplinePt, lastSplinePt)) / tex_scale;
					
					// move texture position
					float tempPos = texturePos.y;
					if (forward)
						tempPos += len;
					else
						tempPos -= len;

					// calculated texture pos is within the given texture, use splinepoint position directly - no rest length  
					if (tempPos <= size.y && tempPos >= 0.0f)
					{
						rest_len = 0.0f;
						texturePos.y = tempPos;

						triangles_[tri_index * 2 + 1].position = spline_->splinePoints_[i].position;
						triangles_[tri_index * 2].position = spline_->splinePoints_[i].position;

						triangles_[tri_index * 2].position += spline_->normals_[i] * width_ / 2.0f;
						triangles_[tri_index * 2 + 1].position -= spline_->normals_[i] * width_ / 2.0f;

						triangles_[tri_index * 2 + 1].texCoords = triangles_[tri_index * 2].texCoords = texturePos;
						triangles_[tri_index * 2 + 1].texCoords.x += texture_seg_size.x;

						trianglesLengths_[tri_index * 2] = trianglesLengths_[tri_index * 2 + 1] = spline_->splinePointsLengths_[i];

						tri_index++;
					}
					// texture pos exceeded the given texture. create triangle point on spline at max texture size - remember rest
					else
					{
						do
						{
							//float rest_until_texture_end = size.y - texturePos.y - len;
							float len_until_texture_end = size.y - texturePos.y - rest_len;
							if (forward == false)
								len_until_texture_end = texturePos.y - rest_len;

							//len_until_texture_end /= tex_scale;

							if (forward)
								texturePos.y = size.y;
							else
								texturePos.y = 0.0f;

							rest_len = fabs(tempPos - texturePos.y);
							
							trianglesLengths_[tri_index * 2] = trianglesLengths_[tri_index * 2 + 1] = spline_->splinePointsLengths_[i] - rest_len;
							float timeatendoftexture = (spline_->splinePointsLengths_[i - 1] + len_until_texture_end*tex_scale) / spline_->getLength();



							triangles_[tri_index * 2 + 1].position = spline_->getLocationAtTime(timeatendoftexture);
							triangles_[tri_index * 2].position = triangles_[tri_index * 2 + 1].position;

							// todo: get normal at time
							triangles_[tri_index * 2].position += spline_->normals_[i] * width_ / 2.0f;
							triangles_[tri_index * 2 + 1].position -= spline_->normals_[i] * width_ / 2.0f;

							triangles_[tri_index * 2 + 1].texCoords = triangles_[tri_index * 2].texCoords = texturePos;
							triangles_[tri_index * 2 + 1].texCoords.x += texture_seg_size.x;

							forward = !forward;
							tri_index++;

							if (rest_len >= size.y)
							{
								tempPos = rest_len - texturePos.y;
								triangles_.resize(triangles_.getVertexCount() + 2);
								trianglesLengths_.resize(triangles_.getVertexCount() + 2);
							}
						} while (rest_len >= size.y); // input another triangle when rest_len reached the texture size						
					}

					//tri_index++;
					lastSplinePt = curSplinePt;
				}


			}
		
			if (rest_len > 0.0f)
			{
				// todo: insert another triangle to reach the last spline point
				// this does not matter too much for the calculation because the end is removed at the next input anyway.
				// it will only miss the rest_len for drawing
				printf("todo: insert another triangle to reach the last spline point. rest_len: %f.\n", rest_len);
			}
}
	}
}