#include "CityGenerator.h"

namespace tgf
{
	namespace utilities
	{
		void CityGenerator::applySettings(cgSettings settings)
		{
			settings_ = settings;
		}
		cgSettings CityGenerator::getSettings()
		{
			return settings_;
		}

		void CityGenerator::generate()
		{
			generateRoads();
		}


		void CityGenerator::generateRoads()
		{
			roadsegment_candidate start;
			start.a = settings_.road_startingPoint;

			start.b = extension_top;
			road_candidates_.push_back(start);
			start.b = extension_left;
			road_candidates_.push_back(start);
			start.b = extension_right;
			road_candidates_.push_back(start);
			start.b = extension_down;
			road_candidates_.push_back(start);

			while (road_candidates_.size())
			{

			}
		}

		void CityGenerator::processRoadSegment(roadsegment_candidate& seg)
		{
			// check wether placement is possible

			// check to split (left right extension)
				// new candidates (apply angle)

			// check to continue (straight extension)
				// new candidate (apply angle)
		}
	}
}