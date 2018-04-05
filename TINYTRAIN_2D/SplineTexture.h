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

			void setTexture(sf::Texture* texture);
			sf::Texture* getTexture();

			int calcTriangleIndexAtSplinePt(int spline_pt_index);
			bool cutTrianglesAtSplineIndex(int spline_index);
			bool cutTrianglesAtIndex(int triangle_index);

			std::unique_ptr<Spline_CatmullRom> spline_;
			float width_;
			// todo: use enum type instead of bool
			bool useSplineptsForTextureSplitting_;

			void createTrianglesFromSpline(int startindex = 0);
			sf::VertexArray& getTriangleData();
		protected:
			sf::Texture* texture_;
			sf::VertexArray triangles_;
			std::vector<float> trianglesLengths_;
			int last_processed_startindex_;
			
			// Inherited via Entity
			virtual void onDraw(sf::RenderTarget * target) override;
			virtual void onUpdate(float deltaTime) override;
			
			//void createTrianglesFromSpline(int startindex);
			void createTriangles_splitTextureByLength(int startindex);
			void createTriangles_splitTextureByPoints(int startindex);
			bool fillTriangleData(int tri_index, int i, sf::Vector2f texturePos);
			bool fillTriangleData_manual(int tri_index, sf::Vector2f pos, sf::Vector2f normal, float len, sf::Vector2f texturePos);
		};
	}
}