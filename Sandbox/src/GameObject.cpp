#include "GameObject.h"

#include "ShooterGame.h"
#include "Level.h"

GameObject::GameObject(const Transform& transform)
	: m_Transform(transform)
{
}

GameObject::GameObject(const glm::vec3& position, const glm::vec2 rotation, const glm::vec2& scale)
{
	m_Transform.position = position;
	m_Transform.rotation = rotation;
	m_Transform.scale = scale;
}

void GameObject::Destroy()
{
	m_bIsActive = false;
	Level* level = GetLevel();
	level->DestroyGameObject(this);
}
