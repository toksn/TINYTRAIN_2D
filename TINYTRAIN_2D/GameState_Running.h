#pragma once
#include "GameStateBase.h"

class GameState_Running : public tgf::GameStateBase
{
public:
	GameState_Running(tgf::Game* game);
	~GameState_Running();

	// Inherited via GameStateBase
	virtual void update(float deltaTime) override;
	virtual void draw(sf::RenderTarget * target) override;
	virtual void handleInput() override;

	void loadLevel();

private:

};