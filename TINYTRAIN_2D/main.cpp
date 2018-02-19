#include <SFML/Graphics.hpp>
#include "Game.h"

int main()
{
	srand (time(NULL));

	tgf::Game game;
	game.restart();
	game.run();

	return 0;
}