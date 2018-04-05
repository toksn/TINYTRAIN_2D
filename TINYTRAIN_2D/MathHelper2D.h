#pragma once
#include "tinyc2.h"

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
		};
	}
}