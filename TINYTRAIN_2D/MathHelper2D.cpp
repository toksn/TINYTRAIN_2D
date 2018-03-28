#include "MathHelper2D.h"

namespace tgf
{
	namespace math
	{
		// Returns 0 if the lines do not intersect or overlap
		// Returns 1 if the lines intersect
		//  Returns 2 if the lines overlap, contain the points where overlapping start starts and stop
		int MathHelper2D::segment_segment_intersection(c2v s1_a, c2v s1_b, c2v s2_a, c2v s2_b, c2v * intersection)
		{
			//http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
			c2v oa, ob, da, db; //Origin and direction vectors
			float sa, sb; //Scalar values
			oa = s1_a;
			da = c2Sub(s1_b, s1_a);
			ob = s2_a;
			db = c2Sub(s2_b, s2_a);
				
			if (c2Det2(da, db) == 0 && (c2Det2(c2Sub(ob, oa),da) == 0)) //If colinear
			{
				if (point_on_segment(s2_b, s1_a, s1_b) || point_on_segment(s2_b, s1_a, s1_b) ||
					point_on_segment(s1_a, s2_a, s2_b) || point_on_segment(s1_b, s2_a, s2_b))
					return 2;
				else
					return 0;

				/*
				if (point_on_segment(s2_b, s1_a, s1_b) && point_on_segment(s2_b, s1_a, s1_b))
				{
					//r.push_back(lb.pa);
					//r.push_back(lb.pb);
					//printf("colinear, overlapping\n");
					return 2;
				}

				if (point_on_segment(s1_a, s2_a, s2_b) && point_on_segment(s1_b, s2_a, s2_b))
				{
					//r.push_back(la.pa);
					//r.push_back(la.pb);
					//printf("colinear, overlapping\n");
					return 2;
				}
					
				if (point_on_segment(s1_a, s2_a, s2_b))
					r.push_back(s1_a);
					
				if (on_segment(la.pb, lb))
					r.push_back(s1_b);

				if (on_segment(lb.pa, la))
					r.push_back(s2_a);

				if (on_segment(lb.pb, la))
					r.push_back(s2_b);

				if (r.size() == 0)
					dprintf("colinear, non-overlapping\n");
				else
					dprintf("colinear, overlapping\n");

				return r;*/
			}

			if (c2Det2(da,db) == 0 && c2Det2(c2Sub(ob,oa),da) != 0)
			{
				//dprintf("parallel non-intersecting\n");
				return 0;
			}

			//Math trick db cross db == 0, which is a single scalar in 2D.
			//Crossing both sides with vector db gives:
			sa = c2Det2(c2Sub(ob,oa),db) / c2Det2(da, db);

			//Crossing both sides with vector da gives
			sb = c2Det2(c2Sub(oa,ob),da) / c2Det2(db,da);

			if (0 <= sa && sa <= 1 && 0 <= sb && sb <= 1)
			{
				//dprintf("intersecting\n");
				//r.push_back(oa + da * sa);
				if (intersection)
					*intersection = c2Add(oa, c2Mulvs(da, sa));
					
				return 1;
			}

			//dprintf("non-intersecting, non-parallel, non-colinear, non-overlapping\n");
			return 0;
		}

		bool MathHelper2D::segment_segment_intersect(c2v s1_a, c2v s1_b, c2v s2_a, c2v s2_b)
		{
			// Find the four orientations needed for general and
			// special cases
			int o1 = segment_point_orientation(s1_a, s1_b, s2_a);
			int o2 = segment_point_orientation(s1_a, s1_b, s2_b);
			int o3 = segment_point_orientation(s2_a, s2_b, s1_a);
			int o4 = segment_point_orientation(s2_a, s2_b, s1_b);

			// General case
			if (o1 != o2 && o3 != o4)
				return true;

			// Special Cases
			// s1_a, s1_b and s2_a are colinear and s2_a lies on segment 1
			if (o1 == 0 && point_on_segment(s2_a, s1_a, s1_b)) return true;

			// s1_a, s1_b and s2_b are colinear and s2_b lies on segment 1
			if (o2 == 0 && point_on_segment(s2_b, s1_a, s1_b)) return true;

			// s2_a, s2_b and s1_a are colinear and s1_a lies on segment 2
			if (o3 == 0 && point_on_segment(s1_a, s2_a, s2_b)) return true;
			
			// s2_a, s2_b and s1_b are colinear and s1_b lies on segment 2
			if (o4 == 0 && point_on_segment(s1_b, s2_a, s2_b)) return true;

			return false; // Doesn't fall in any of the above cases
		}


		// To find orientation of ordered triplet (s_a, s_b, pt).
		// The function returns following values
		// 0 --> s_a, s_b and pt are colinear
		// 1 --> Clockwise
		// 2 --> Counterclockwise
		int MathHelper2D::segment_point_orientation(c2v s_a, c2v s_b, c2v pt)
		{
			// See 10th slides from following link for derivation of the formula
			// http://www.dcs.gla.ac.uk/~pat/52233/slides/Geometry1x1.pdf
			int val = (s_b.y - s_a.y) * (pt.x - s_b.x) -
				(s_b.x - s_a.x) * (pt.y - s_b.y);

			if (val == 0) return 0;  // colinear

			return (val > 0) ? 1 : 2; // clock or counterclock wise
		}
		
		// this function only works at the moment when the three points are colinear!
		// todo: further checks wether pt is on line of a->b
		bool MathHelper2D::point_on_segment(c2v pt, c2v s_a, c2v s_b)
		{
			// this requires points to be colinear:
			//
			//if (pt.x <= c2Max(s_a.x, s_b.x) && pt.x >= c2Min(s_a.x, s_b.x) &&
			//	pt.y <= c2Max(s_a.y, s_b.y) && pt.y >= c2Min(s_a.y, s_b.y))
			//	return true;
			//
			//return false;

			float da = c2Distance(pt, s_a);
			float db = c2Distance(pt, s_b);
			float line = c2Distance(s_a, s_b);
			if ((da + db) > line /*+eps*/)
				return false;

			return true;
		}
	}
}