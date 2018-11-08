#pragma once
#include "tinyc2.h"
#include <utility>

namespace tgf
{
	namespace math
	{
		C2_INLINE float c2Distance(c2v a, c2v b) { return c2Len(c2Sub(b, a)); }
		C2_INLINE bool c2Equal(c2v a, c2v b) { return a.x == b.x && a.y == b.y; }
		
		class MathHelper2D
		{
		public:
			// segment to segment intersection with intersection point calculation
			static int segment_segment_intersection(c2v s1_a, c2v s1_b, c2v s2_a, c2v s2_b, c2v* intersection = NULL);
			// faster segment to segment intersection to just check for an intersection
			static bool segment_segment_intersect(c2v s1_a, c2v s1_b, c2v s2_a, c2v s2_b);
			
			static int segment_point_orientation(c2v s_a, c2v s_b, c2v pt);
			static bool point_on_segment(c2v pt, c2v s_a, c2v s_b);
			static bool ray_to_segment_intersection(c2v rayOrigin, c2v rayDirection, c2v s_a, c2v s_b, c2v * intersection = NULL);
			//static float point_to_segment_distance(c2v pt, c2v s_a, c2v s_b);

			static c2v calc_point_on_circle(float r, float angle, c2v c = { 0.0f, 0.0f });
			static c2AABB calc_aabb(void* shape, C2_TYPE type);

			static std::pair<int, int> getArrayCoordsFromIndex(int index, int row_len)
			{
				int y = index / row_len;
				int x = index - y * row_len;
				return std::make_pair(x,y);
			};

			// this function converts any number of 90° degree rotations into up to three mirroring operations
			// in: rot in number of 90° rectangular rotations, can be positive and negative
			// in/out: initial and resulting mirror_horizontally operation
			// in/out: initial and resulting mirror_vertically operation
			// in/out: initial and resulting mirror_diagonally operation
			static void convertRectangularRotationToMirrorOps(int rectangular_rotation, bool& mirror_horizontally, bool& mirror_vertically, bool& mirror_diagonally);
			
			/*
			static sf::IntRect mirror_rect(sf::IntRect& rect, bool h, bool v)
			{
				if(h && v)
					return sf::IntRect(rect.left + rect.width, rect.top + rect.height, rect.width*-1.0f, rect.height*-1.0f);		// mirror both
				else if(h)
					return sf::IntRect(rect.left + rect.width, rect.top, rect.width*-1.0f, rect.height);	// mirror horizontally
				else if(v)
					return sf::IntRect(rect.left, rect.top + rect.height, rect.width, rect.height*-1.0f);	// mirror vertically
				
				return rect;
			}*/

		};
	}
}