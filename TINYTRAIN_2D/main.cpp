#include <SFML/Graphics.hpp>
#include "Game.h"
#include "GameState_Running.h"
#include "GameState_MainMenu.h"

int main()
{
	srand (time(NULL));

	tgf::Game game("tinytrain 2D", 600, 400, 100);

	game.loadFont("data/fonts/pixantiqua.ttf");
	game.loadTextureAtlas("data/images/texture_atlas.png");
	
	//std::unique_ptr<tgf::GameStateBase> state(new GameManager(&game));
	
	//game.states_.push_back(std::make_unique<tinytrain::GameState_Running>(&game));
	game.pushState(std::make_unique<tinytrain::GameState_MainMenu>(&game));
	game.run();

	return 0;
}