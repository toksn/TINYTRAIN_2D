#pragma once
#include "GameStateBase.h"

class GameManager : public tgf::GameStateBase
{
public:
	GameManager(tgf::Game* game);
	~GameManager();

	// Inherited via GameStateBase
	virtual void update(float deltaTime) override;
	virtual void draw(sf::RenderTarget * target) override;
	virtual void handleInput() override;

	void loadLevel();

private:

};