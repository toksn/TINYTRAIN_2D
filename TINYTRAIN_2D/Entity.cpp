#include "Entity.h"
#include "GameManager.h"

Entity::Entity(EntityManager * man)
{
	m_manager = man;
}

Entity::~Entity()
{
}

void Entity::draw(sf::RenderWindow & target)
{
	if (m_manager && m_transformable)
	{
		auto screenTransformation = m_manager->convertToScreen(m_pos);
		*m_transformable = screenTransformation;

		//m_transformable->setPosition(screenPos);
		//float angle = angle_rad * 180.0f / b2_pi;
		//m_transformable->setRotation(mPos->GetAngle() * 180.0f / sf::);
	}
}

//void Entity::setPosition(sf::Vector2f pos)
//{
//	m_pos.setPosition(pos)
//}
//
//void Entity::setAngle(float angle)
//{
//	if (m_body)
//		m_body->SetTransform(m_body->GetPosition(), angle);
//}
//
//void Entity::setTransform(b2Vec2 pos, float angle)
//{
//	if (m_body)
//		m_body->SetTransform(pos, angle);
//}
