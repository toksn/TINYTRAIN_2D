#include "tinyc2.h"
#include "TLevel.h"
#include "TTrain.h"
#include "TRailRoad.h"

namespace tinytrain
{
	TLevel::TLevel()
	{
	}


	TLevel::~TLevel()
	{
	}

	void TLevel::draw(sf::RenderTarget * target)
	{
		if (m_train)
			m_train->draw(target);
		if (m_railroad)
			m_railroad->draw(target);
	}

	void TLevel::update(float deltaTime)
	{
		if (m_train)
			m_train->update(deltaTime);
		if (m_railroad)
			m_railroad->update(deltaTime);
	}

	void TLevel::load(std::string file)
	{
		if (file.empty())
		{
			/**************************************************************************
			SIMPLE LEVEL CREATED BY CODE -- this is the minimum requirement for a level
			***************************************************************************/
			// create train for the player
			m_train = std::make_unique<TTrain>();

			// create a railroad for the train
			m_railroad = std::make_unique<TRailRoad>();

			m_railroad->append(sf::Vector2f(200.0f, 50.f));
			m_railroad->append(sf::Vector2f(200.0f, 100.f));
			m_railroad->append(sf::Vector2f(250.0f, 140.f));
			m_railroad->append(sf::Vector2f(150.0f, 180.f));
			m_railroad->append(sf::Vector2f(130.0f, 70.f));

			c2v start{ 150.0f, 180.f };
			c2v end{ 130.0f, 70.f };
			float dist = 10.0f;
			int angle_range = 20;
			angle_range *= 100;

			c2v seg = c2Sub(end, start);
			// 57.295779513 := rad to degre conversion (rad * 180.0/pi)
			float angle = atan2(seg.y, seg.x) * 57.295779513f;

			for (size_t i = 0; i < 500; i++)
			{
				angle += ((rand() % angle_range) - angle_range * 0.5f)/100.0f;

				//lastPos.x += rand() % 200 - 100;
				//lastPos.y += rand() % 200 - 100;

				//lastPos.x += rand() % 30;
				//lastPos.y += rand() % 30;

				start = end;
				end.x += dist * cos(angle / 57.295779513f);
				end.y += dist * sin(angle / 57.295779513f);
				
				m_railroad->append(sf::Vector2f(end.x, end.y));
			}

			m_railroad->addTrain(m_train.get());
			m_train->initWagons(30);

			// create obstacles for the games to be lost
			//create<TT_Obstacle>();

			// create target zone for the game to be won
			//create<TT_TargetZone>();
			/************************************************************************/

			// TODO:
			// passengers to pick up
		}
	}
}