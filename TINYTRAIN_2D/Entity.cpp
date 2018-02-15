#include "Entity.h"
#include "GameManager.h"

Entity::Entity(EntityManager * man)
{
	m_manager = man;
}

Entity::~Entity()
{
}