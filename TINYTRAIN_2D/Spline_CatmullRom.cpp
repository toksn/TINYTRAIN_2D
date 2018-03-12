#include "Spline_CatmullRom.h"
#include "tinyc2.h"

namespace tgf
{
	namespace math
	{
		Spline_CatmullRom::Spline_CatmullRom()
		{
			pointsPerSegment_ = 20;
			type_ = CatmullRomType::Centripetal;

			interpolateControlPointEnds_ = false;
		}

		Spline_CatmullRom::~Spline_CatmullRom()
		{
		}

		void Spline_CatmullRom::onControlPointsAdded(int a_startindex)
		{
			int controlPointCount = controlPoints_.getVertexCount();
			int min_start_index = 3;
			if (interpolateControlPointEnds_)
			{
				controlPointCount++;
				min_start_index--;
			}				
			
			if (controlPointCount < 4)
			{
				return;
			}


			if (a_startindex < min_start_index)
				a_startindex = min_start_index;

			// remove spline points that already exist but are after the current startindex
			// this enable recalculation of the spline from startindex on
			int expected_spline_point_count = pointsPerSegment_ * (a_startindex- min_start_index);
			if (splinePoints_.getVertexCount() > expected_spline_point_count)
			{
				splinePointsLengths_.resize(expected_spline_point_count);
				splinePoints_.resize(expected_spline_point_count);
			}

			sf::Vector2f pts[4];
			while (a_startindex <= controlPointCount-1)
			{
				int new_control_point_index = a_startindex++;
				int pt = new_control_point_index - 2;				
				
				if (interpolateControlPointEnds_ == false)
				{
					pts[0] = controlPoints_[pt - 1].position;
					pts[1] = controlPoints_[pt].position;
					pts[2] = controlPoints_[pt + 1].position;
					pts[3] = controlPoints_[pt + 2].position;
				}
				else
				{
					if (pt - 1 < 0)
					{
						c2v pt = c2Lerp(c2v{ controlPoints_[0].position.x,controlPoints_[0].position.y }, c2v{ controlPoints_[1].position.x,controlPoints_[1].position.y }, -1.0f);
						pts[0] = { pt.x, pt.y };
					}
					else
						pts[0] = controlPoints_[pt - 1].position;
					pts[1] = controlPoints_[pt].position;
					pts[2] = controlPoints_[pt + 1].position;
					if (pt + 2 > controlPoints_.getVertexCount() - 1)
					{
						int lastindex = controlPoints_.getVertexCount() - 1;
						c2v pt = c2Lerp(c2v{ controlPoints_[lastindex - 1].position.x,controlPoints_[lastindex - 1].position.y }, c2v{ controlPoints_[lastindex].position.x,controlPoints_[lastindex].position.y }, 2.0f);
						pts[3] = { pt.x, pt.y };
					}
					else
						pts[3] = controlPoints_[pt + 2].position;
				}				

				for (int i = 0; i < pointsPerSegment_; i++)
				{
					if (type_ == CatmullRomType::Uniform)
					{
						float u = (float)i / (float)(pointsPerSegment_-1);
						appendSplinePoint(interpolateUniform(u, pts[0], pts[1], pts[2], pts[3]));
					}
					else
					{
						// for this calculation, see Definition section of https://en.wikipedia.org/wiki/Centripetal_Catmull%E2%80%93Rospline_
						float* x = new float[4]{ pts[0].x, pts[1].x, pts[2].x, pts[3].x };
						float* y = new float[4]{ pts[0].y, pts[1].y, pts[2].y, pts[3].y };
						float* time = new float[4]{ 0, 1, 2, 3 };

						float tstart = 1.0f;
						float tend = 2.0f;

						float total = 0.0f;
						for (int j = 1; j < 4; j++) 
						{
							float dx = x[j] - x[j - 1];
							float dy = y[j] - y[j - 1];
							if (type_ == CatmullRomType::Centripetal)
								total += pow(dx * dx + dy * dy, .25);
							else //if(type_ == CatmullRomType::Chordal)
								total += pow(dx * dx + dy * dy, .5);
							time[j] = total;
						}
						tstart = time[1];
						tend = time[2];

						float u = tstart + (i * (tend - tstart)) / (float)(pointsPerSegment_-1);
						//float u = (float)i / (float)pointsPerSegment_;
						appendSplinePoint(interpolate(u, pts[0], pts[1], pts[2], pts[3], time));
						delete[] x;
						delete[] y;
						delete[] time;
					}
				}
			}
		}

		sf::Vector2f Spline_CatmullRom::interpolateUniform(float u, sf::Vector2f pt0, sf::Vector2f pt1, sf::Vector2f pt2, sf::Vector2f pt3)
		{
			float u2 = u * u;
			float u3 = u2 * u;

			float f1 = -0.5 * u3 + 1.0 * u2 - 0.5 * u;
			float f2 =  1.5 * u3 - 2.5 * u2				+ 1.0;
			float f3 = -1.5 * u3 + 2.0 * u2 + 0.5 * u;
			float f4 =  0.5 * u3 - 0.5 * u2;

			sf::Vector2f point;
			point.x = pt0.x * f1 + pt1.x * f2 + pt2.x * f3 + pt3.x * f4;
			point.y = pt0.y * f1 + pt1.y * f2 + pt2.y * f3 + pt3.y * f4;
			return point;
		}

		// calculation is based on figure 3 of http://www.cemyuksel.com/research/catmullroparam_/catmullrom.pdf
		// uniform time vector should be the same as interpolateUniform(..) but slower because of the additional calcs
		// 
		// param: float* time is a float array of 4 entries, memory managment outside, no checks will be done in this func!
		sf::Vector2f Spline_CatmullRom::interpolate(float u, sf::Vector2f pt0, sf::Vector2f pt1, sf::Vector2f pt2, sf::Vector2f pt3, float* time)
		{
			float L01 = pt0.x * (time[1] - u) / (time[1] - time[0]) + pt1.x * (u - time[0]) / (time[1] - time[0]);
			float L12 = pt1.x * (time[2] - u) / (time[2] - time[1]) + pt2.x * (u - time[1]) / (time[2] - time[1]);
			float L23 = pt2.x * (time[3] - u) / (time[3] - time[2]) + pt3.x * (u - time[2]) / (time[3] - time[2]);
			float L012 = L01 * (time[2] - u) / (time[2] - time[0]) + L12 * (u - time[0]) / (time[2] - time[0]);
			float L123 = L12 * (time[3] - u) / (time[3] - time[1]) + L23 * (u - time[1]) / (time[3] - time[1]);
			float C12 = L012 * (time[2] - u) / (time[2] - time[1]) + L123 * (u - time[1]) / (time[2] - time[1]);

			float yL01 = pt0.y * (time[1] - u) / (time[1] - time[0]) + pt1.y * (u - time[0]) / (time[1] - time[0]);
			float yL12 = pt1.y * (time[2] - u) / (time[2] - time[1]) + pt2.y * (u - time[1]) / (time[2] - time[1]);
			float yL23 = pt2.y * (time[3] - u) / (time[3] - time[2]) + pt3.y * (u - time[2]) / (time[3] - time[2]);
			float yL012 = yL01 * (time[2] - u) / (time[2] - time[0]) + yL12 * (u - time[0]) / (time[2] - time[0]);
			float yL123 = yL12 * (time[3] - u) / (time[3] - time[1]) + yL23 * (u - time[1]) / (time[3] - time[1]);
			float yC12 = yL012 * (time[2] - u) / (time[2] - time[1]) + yL123 * (u - time[1]) / (time[2] - time[1]);

			return sf::Vector2f(C12, yC12);
		}
	}
}