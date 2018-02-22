#include "GameStateBase.h"

namespace tgf
{
	void GameStateBase::handleInput(sf::Event & e)
	{
		for (auto f : eventCallbacks_[e.type])
			f.first(e);
	}
}