#pragma once

#include "Entity.h"
#include "Spline_CatmullRom.h"
using namespace tgf::math;

namespace tgf
{
	namespace utilities
	{
		class SplineTexture : public Entity
		{
		public:
			SplineTexture();
			~SplineTexture();

			std::unique_ptr<Spline_CatmullRom> spline_;
			float width_;
		protected:
			sf::Texture texture_;
			sf::VertexArray triangles_;
			std::vector<float> trianglesLengths_;
			int last_processed_startindex_;

			// Inherited via Entity
			virtual void onDraw(sf::RenderTarget * target) override;
			virtual void onUpdate(float deltaTime) override;

			void createTriangleStripFromSpline(int startindex = 0);
			void createTriangleStrip_splitTextureByLength(int startindex);
			void createTriangleStrip_splitTextureByPoints(int startindex);
		};
	}
}