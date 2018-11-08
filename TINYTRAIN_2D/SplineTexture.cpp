#include "SplineTexture.h"
#include "tinyc2.h"

namespace tgf
{
	namespace utilities
	{
		SplineTexture::SplineTexture() : Entity() 
		{
			spline_ = std::make_unique<Spline_CatmullRom>();
			
			width_ = -1.0f;
			useSplineptsForTextureSplitting_ = false;

			// spline texture can deal with sf::PrimitiveType::Triangles and sf::PrimitiveType::TriangleStrip
			//
			// sf::PrimitiveType::TriangleStrip is more efficient, however in some situations it may be 
			// desired to generate sf::PrimitiveType::Triangles data. 
			// (for example to include the data in another bigger structure, based on Triangles)
			triangles_.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
			//triangles_.setPrimitiveType(sf::PrimitiveType::Triangles);

			last_processed_startindex_ = -1;
		}

		SplineTexture::~SplineTexture()
		{
		}

		void SplineTexture::setTexture(sf::Texture * texture)
		{
			texture_ = texture;
			if (texture)
				width_ = texture->getSize().x;
		}

		sf::Texture * SplineTexture::getTexture()
		{
			return texture_;
		}

		int SplineTexture::calcTriangleIndexAtSplinePt(int spline_pt_index)
		{
			int index_multi = 2;
			if (triangles_.getPrimitiveType() == sf::PrimitiveType::Triangles)
				index_multi = 6;
			
			int tri_index = spline_pt_index * index_multi + 1;

			if (useSplineptsForTextureSplitting_ == false)
			{
				if (spline_ && spline_->splinePointsLengths_.size() >= spline_pt_index && spline_pt_index > 1)
				{
					float startlen = spline_->splinePointsLengths_[spline_pt_index - 1];
					for (; tri_index < trianglesLengths_.size(); tri_index+= index_multi)
					{
						if (trianglesLengths_[tri_index] > startlen)
							break;
					}
				}
			}

			return tri_index;			
		}

		// remove all triangles from and including the given spline_index
		bool SplineTexture::cutTrianglesAtSplineIndex(int spline_index)
		{
			int tri_index = calcTriangleIndexAtSplinePt(spline_index);

			if (triangles_.getPrimitiveType() == sf::PrimitiveType::TriangleStrip)
				tri_index -= 1;
			else if (triangles_.getPrimitiveType() == sf::PrimitiveType::Triangles)
				tri_index -= 4;

			if (tri_index < 0)
				tri_index = 0;

			bool rc = cutTrianglesAtIndex(tri_index);
			
			if(rc)
				last_processed_startindex_ = spline_index - 1;

			return rc;
		}

		// remove all triangles from and including the given triangle_index
		bool SplineTexture::cutTrianglesAtIndex(int triangle_index)
		{
			bool rc = false;
			if (triangle_index < triangles_.getVertexCount() && triangle_index >= 0)
			{
				trianglesLengths_.resize(triangle_index);
				triangles_.resize(triangle_index);
				rc = true;
			}
			return rc;
		}

		void SplineTexture::onDraw(sf::RenderTarget * target)
		{	
			sf::RenderStates state;
			state.texture = texture_;
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
				createTrianglesFromSpline(startindex);
			
				// update last processed startindex
				last_processed_startindex_ = spline_->startIndex_lastUpdate_;
			}
		}

		void SplineTexture::createTrianglesFromSpline(int startindex)
		{
			if (useSplineptsForTextureSplitting_)
				createTriangles_splitTextureByPoints(startindex);
			else
				createTriangles_splitTextureByLength(startindex);
		}

		sf::VertexArray & SplineTexture::getTriangleData()
		{
			return triangles_;
		}

		// the texture will get split depending on the spline segment lengths.
		// this can be used when you want your texture to be represented as its 
		// actual content very closely. good for repeating textures like streets/rails.
		//
		//	options:
		//	width_ = output width of the texture that is following the spline.
		//				the height will get scaled accordingly to maintain the correct texture aspect ratio.
		//	...
		void SplineTexture::createTriangles_splitTextureByLength(int startindex)
		{
			if (spline_ == nullptr || spline_->splinePoints_.getVertexCount() == 0 || startindex >= spline_->splinePoints_.getVertexCount())
				return;

			auto size = texture_->getSize();
			float tex_scale = width_ / size.x;
			float rest_len = 0.0f;
			int tri_index = calcTriangleIndexAtSplinePt(startindex); // startindex;
			//sf::Color cols[3] = { sf::Color::Red, sf::Color::Green, sf::Color::Blue };			
			c2v lastSplinePt = { spline_->splinePoints_[startindex].position.x, spline_->splinePoints_[startindex].position.y };
			sf::Vector2f texturePos(0.0f, 0.0f);

			bool forward = true;
			int index_multi = 2;
			if (triangles_.getPrimitiveType() == sf::PrimitiveType::Triangles)
				index_multi = 6;

			if (startindex > 0)
			{
				lastSplinePt = { spline_->splinePoints_[startindex - 1].position.x, spline_->splinePoints_[startindex - 1].position.y };

				// find tri_index to start from
				//tri_index = calcTriangleIndexAtSplinePt(startindex);
				float startlen = spline_->splinePointsLengths_[startindex - 1];

				int lastvert_index = tri_index - index_multi;

				sf::Vertex lastVert = triangles_[lastvert_index];
				texturePos.y = lastVert.texCoords.y;
				if (lastVert.texCoords.y == 0.0f)
				{
					forward = true;
					rest_len = startlen - trianglesLengths_[lastvert_index];
				}
				else if (lastVert.texCoords.y == size.y)
				{
					forward = false;
					rest_len = startlen - trianglesLengths_[lastvert_index];
				}
				else if (startindex > 1)
				{
					forward = lastVert.texCoords.y > triangles_[lastvert_index - index_multi].texCoords.y;

					rest_len = startlen - trianglesLengths_[lastvert_index];
					if (rest_len != 0.0f)
					{
						printf("todo: rest_len calc is probably wrong. rest: %f\n", rest_len);
					}
				}
			}

			int triangles_to_generate = 1 + tri_index;//+ calcTriangleIndexAtSplinePt(spline_->splinePoints_.getVertexCount() - 1);
			triangles_to_generate += (spline_->splinePoints_.getVertexCount() - 1 - startindex)*index_multi;

			tri_index /= index_multi;
			int diff = tri_index - startindex;
			if (diff)
				printf("difference = %i (tri_index = %i, start_index = %i)\n", diff, tri_index, startindex);

			
			triangles_.resize(triangles_to_generate);
			trianglesLengths_.resize(triangles_to_generate);
			//replaced: trianglesLengths_.resize((spline_->splinePoints_.getVertexCount() + tri_index - startindex) * index_multi);

			// use spline segment len for texturesplitting
			for (int i = startindex; i < spline_->splinePoints_.getVertexCount(); i++)
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

					fillTriangleData(tri_index, i, texturePos);

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

						if (forward)
							texturePos.y = size.y;
						else
							texturePos.y = 0.0f;

						rest_len = fabs(tempPos - texturePos.y);

						float timeatendoftexture = (spline_->splinePointsLengths_[i - 1] + len_until_texture_end * tex_scale) / spline_->getLength();
						auto pos = spline_->getLocationAtTime(timeatendoftexture);
						auto normal = spline_->normals_[i] * width_ / 2.0f;
						auto len = spline_->splinePointsLengths_[i] - rest_len;
						fillTriangleData_manual(tri_index, pos, normal, len, texturePos);
												
						forward = !forward;
						tri_index++;

						if (rest_len >= size.y)
						{
							tempPos = rest_len - texturePos.y;
							triangles_.resize(triangles_.getVertexCount() + index_multi);
							trianglesLengths_.resize(triangles_.getVertexCount() + index_multi);
						}
					} while (rest_len >= size.y); // input another triangle when rest_len reached the texture size						
				}

				//tri_index++;
				lastSplinePt = curSplinePt;
			}

			if (rest_len > 0.0f)
			{
				// todo: insert another triangle to reach the last spline point
				// this does not matter too much for the calculation because the end is removed at the next input anyway.
				// it will only miss the rest_len for drawing
				//printf("todo: insert another triangle to reach the last spline pt. rest_len: %f.\n", rest_len);
			}
		}


		// the texture will get split uniformly between splinepoints.
		//
		//	options:
		//	cuts.y - number of spline pts that include one full texture height
		//	...
		void SplineTexture::createTriangles_splitTextureByPoints(int startindex)
		{
			auto size = texture_->getSize();
			sf::Vector2f texturePos(0.0f, 0.0f);
			c2v lastSplinePt = { spline_->splinePoints_[startindex].position.x, spline_->splinePoints_[startindex].position.y };
			if (startindex > 0)
				lastSplinePt = { spline_->splinePoints_[startindex - 1].position.x, spline_->splinePoints_[startindex - 1].position.y };

			triangles_.resize(spline_->splinePoints_.getVertexCount() * 6);

			// **********************************************************
			// SETTINGS
			// **********************************************************
			// using splinepoints for uniform texture splitting
			sf::Vector2u texture_cuts(1, 2);
			sf::Vector2f texture_seg_size((float)size.x / texture_cuts.x, (float)size.y / texture_cuts.y);
			// **********************************************************

			for (int i = startindex; i < spline_->splinePoints_.getVertexCount(); i++)
			{
				/*cuts: 4

				index		0  1  2  3		4  5  6  7			8

				tex.y (32)	0  8 16 24		32 24 16 8			0

				simple index multiplication	   40 48 56			0
				*/

				// pretty complicated way of doing a two way travel 0->1->0 (see example above to how it pans out)
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

				fillTriangleData(i, i, texturePos);
			}
		}

		// filling triangle data with respect to the primitive type.
		// this function always uses the full width of the given texture (no x axis segmenting)
		bool SplineTexture::fillTriangleData(int tri_index, int spline_index, sf::Vector2f texturePos)
		{
			auto pos = spline_->splinePoints_[spline_index].position;
			auto normal = spline_->normals_[spline_index] * width_ / 2.0f;
			auto len = spline_->splinePointsLengths_[spline_index];
			return fillTriangleData_manual(tri_index, pos, normal, len, texturePos);
		}


		bool SplineTexture::fillTriangleData_manual(int tri_index, sf::Vector2f pos, sf::Vector2f normal, float len, sf::Vector2f texturePos)
		{
			auto size = texture_->getSize();
			float tex_scale = width_ / size.x;

			if (triangles_.getPrimitiveType() == sf::PrimitiveType::TriangleStrip)
			{
				triangles_[tri_index * 2 + 1].position = pos;
				triangles_[tri_index * 2].position = pos;

				triangles_[tri_index * 2].position += normal;
				triangles_[tri_index * 2 + 1].position -= normal;

				triangles_[tri_index * 2 + 1].texCoords = triangles_[tri_index * 2].texCoords = texturePos;
				triangles_[tri_index * 2 + 1].texCoords.x += size.x;

				trianglesLengths_[tri_index * 2] = len;
				trianglesLengths_[tri_index * 2 + 1] = len;
			}
			else if (triangles_.getPrimitiveType() == sf::PrimitiveType::Triangles)
			{
				// create triangles for the spline (1 new points *3, 1 new point * 2 = 5 new points)
				triangles_[tri_index * 6].position = pos;
				triangles_[tri_index * 6].position += normal;
				triangles_[tri_index * 6 + 1].position = pos;
				triangles_[tri_index * 6 + 1].position -= normal;

				// new texture position
				triangles_[tri_index * 6].texCoords = texturePos;
				triangles_[tri_index * 6 + 1].texCoords = texturePos;
				triangles_[tri_index * 6 + 1].texCoords.x += size.x;

				trianglesLengths_[tri_index * 6 - 0] = len;
				trianglesLengths_[tri_index * 6 + 1] = len;

				if (tri_index > 0)
				{
					// 6	--> 2, 3, 4, 5, 6, 7
								
					//		--> -4,-3,-2,-1,0,+1,
					
					// copy vertices
					triangles_[tri_index * 6 - 1] = triangles_[tri_index * 6 + 1];

					triangles_[tri_index * 6 - 2] = triangles_[tri_index * 6];
					triangles_[tri_index * 6 - 4] = triangles_[tri_index * 6];

					// take old triangle points to fill the triangle info (1 old point)
					triangles_[tri_index * 6 - 3] = triangles_[tri_index * 6 - 5];

					// note lenghts
					trianglesLengths_[tri_index * 6 - 3] = trianglesLengths_[tri_index * 6 - 5];

					trianglesLengths_[tri_index * 6 - 4] = len;
					trianglesLengths_[tri_index * 6 - 2] = len;					
					trianglesLengths_[tri_index * 6 - 1] = len;
				}
			}
			else
				return false;

			return true;
		}
	}
}