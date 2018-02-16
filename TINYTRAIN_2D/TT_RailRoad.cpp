#include "TT_RailRoad.h"

#define TINYC2_IMPLEMENTATION
#include "tinyc2.h"

TT_RailRoad::TT_RailRoad()
{
	// default railroad draw type
	setPrimitiveType(sf::PrimitiveType::Lines);
}

TT_RailRoad::~TT_RailRoad()
{
}

void TT_RailRoad::recalcLength(unsigned int startindex)
{
	int size = getVertexCount();
	m_length.resize(size);
	
	// first point always has length zero
	if (startindex == 0)
	{
		m_length[startindex] = 0;
		startindex++;
	}

	// we can assume here that the startindex is > 0 because of the above
	for (int i = startindex; i < size; i++)
		m_length[i] = m_length[i-1] + c2Len(c2v{ this->operator[](i).position.x , this->operator[](i).position.y });
}

void TT_RailRoad::append(const sf::Vertex & vertex)
{
	sf::VertexArray::append(vertex);

	// calc length for the last vertex
	recalcLength(getVertexCount() - 1);
}

float TT_RailRoad::getLength()
{
	if (m_length.size() != getVertexCount())
		recalcLength();

	return m_length[getVertexCount()-1];
}

sf::Vector2f TT_RailRoad::getPositionOnRail(float a_dist)
{
	sf::Vector2f pos;

	//todo: extrapolate first segment to negative distance
	if (a_dist < 0.0f)
		a_dist = 0.0f;

	int size = getVertexCount();
	if (size)
	{
		float len = getLength();

		//todo: extrapolate last segment to over actual railroad length
		if (a_dist > len)
			a_dist = len;

		// make an index guess
		int i = ((float)size * a_dist / len) - 1.0f;

		// keep index range
		while (i > 0 && i < size - 1)
		{
			
			if (m_length[i] > a_dist)
			{
				// found correct segment
				if (m_length[i - 1] <= a_dist)
				{
					if (m_length[i - 1] == a_dist)
					{
						pos = this->operator[](i - 1).position;
					}
					else
					{
						float seg_len = m_length[i] - m_length[i - 1];
						float alpha_on_seg = (a_dist - m_length[i - 1]) / seg_len;

						c2v start{	this->operator[](i - 1).position.x,		this->operator[](i - 1).position.y };
						c2v end{	this->operator[](i).position.x,			this->operator[](i).position.y };
						c2v temp = c2Lerp(start, end, alpha_on_seg);
						pos.x = temp.x;
						pos.y = temp.y;
					}
					// found position
					break;
				}
				else
					i--;
			}
			else
				i++;
		}
	}

	return pos;
}
