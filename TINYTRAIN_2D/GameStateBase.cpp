#include "GameStateBase.h"

namespace tgf
{
	void GameStateBase::handleInput(sf::Event & e)
	{
		for (auto f : m_eventCallbacks[e.type])
			f(e);
	}
}