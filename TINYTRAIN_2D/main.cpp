#include <SFML/Graphics.hpp>
#include "Game.h"
#include "GameState_Running.h"

int main()
{
	srand (time(NULL));

	tgf::Game game("tinytrain 2D", 600, 400, 100);
	
	//std::unique_ptr<tgf::GameStateBase> state(new GameManager(&game));
	
	game.m_states.push(std::make_unique<tinytrain::GameState_Running>(&game));
	game.run();

	return 0;
}