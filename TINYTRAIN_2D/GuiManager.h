#pragma once
#include "EntityManager.h"
class GuiManager :
	public EntityManager
{
public:
	GuiManager();
	~GuiManager();

	// Inherited via EntityManager
	virtual void update(float deltaTime) override;
};

