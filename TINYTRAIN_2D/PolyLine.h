#pragma once
#include <vector>
#include <SFML\Graphics.hpp>

namespace tgf
{
	namespace math
	{
		//template<class T>
		class PolyLine
		{
		public:
			PolyLine();
			~PolyLine();

			sf::Vector2f getLocationAtTime(float a_time);
			sf::Vector2f getLocationAtTime(float a_time, int& hintindex);
			float getDirectionAngleAtTime(float a_time, bool a_in_radiant = true);
			float getDirectionAngleAtTime(float a_time, int& hintindex, bool a_in_radiant = true);

			float getLength();
			void recalcLength(unsigned int startindex = 0);
			float getLengthAtTime(float a_time);


			int getSegmentStartIndexAtTime(float a_time, int indexHint = -1);
			int getSegmentStartIndexAtDist(float a_dist, int indexHint = -1);
			//void clear();

			std::vector<sf::Vector2f> poly_;
			std::vector<float> lengths_;
		private:
		};
	}
}