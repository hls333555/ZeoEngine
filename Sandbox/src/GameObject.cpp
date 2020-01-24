#include "GameObject.h"

#include "ShooterGame.h"
#include "Level.h"

void BoxCollisionData::UpdateData()
{
	if (ownerObject)
	{
		center = glm::vec2({ ownerObject->GetPosition().x, ownerObject->GetPosition().y }) + centerOffset;
		extents = ownerObject->GetScale() / 2.0f * extentsMultiplier;
	}
}

void SphereCollisionData::UpdateData()
{
	if (ownerObject)
	{
		center = glm::vec2({ ownerObject->GetPosition().x, ownerObject->GetPosition().y }) + centerOffset;
		// Use object's xScale as base radius
		radius = ownerObject->GetScale().x / 2.0f * radiusMultiplier;
	}
}

GameObject::GameObject(const Transform& transform)
	: m_Transform(transform)
{
}

GameObject::GameObject(const glm::vec3& position, float rotation, const glm::vec2& scale)
{
	m_Transform.position = position;
	m_Transform.rotation = rotation;
	m_Transform.scale = scale;
}

GameObject::~GameObject()
{
	delete m_CollisionData;
}

void GameObject::Destroy()
{
	m_bIsActive = false;
	Level* level = GetLevel();
	level->DestroyGameObject(this);
}

void GameObject::Reset()
{
	OverlappedObject = nullptr;
}

void GameObject::SetBoxCollisionData(const glm::vec2& extentsMultiplier, const glm::vec2& centerOffset)
{
	if (m_CollisionData)
	{
		ZE_WARN("Failed trying to regenerate box collision data!");
	}
	else
	{
		m_CollisionData = new BoxCollisionData(this, centerOffset, extentsMultiplier);
	}
}

void GameObject::SetSphereCollisionData(float radiusMultiplier, const glm::vec2& centerOffset)
{
	if (m_CollisionData)
	{
		ZE_WARN("Failed trying to regenerate sphere collision data!");
	}
	else
	{
		m_CollisionData = new SphereCollisionData(this, centerOffset, radiusMultiplier);
	}
}

void GameObject::DoCollisionTest(const std::vector<GameObject*>& objects)
{
	for (uint32_t i = 0; i < objects.size(); ++i)
	{
		if (objects[i] == this)
			continue;

		if (objects[i]->IsActive() && objects[i]->IsCollisionEnabled())
		{
			if (CheckCollision(objects[i]))
			{
				// TODO: For now, this GameObject can only overlap one GameObject ONCE!
				if (OverlappedObject != objects[i])
				{
					OverlappedObject = objects[i];
					OnOverlap(objects[i]);
				}
			}
		}
	}
}

bool GameObject::CheckCollision(GameObject* other)
{
	if (GetCollisionType() == ObjectCollisionType::Box)
	{
		if (other->GetCollisionType() == ObjectCollisionType::Box)
		{
			return CheckCollision_BB(other);
		}
		else if (other->GetCollisionType() == ObjectCollisionType::Sphere)
		{
			return CheckCollision_BS(this, other);
		}
	}
	else if (GetCollisionType() == ObjectCollisionType::Sphere)
	{
		if (other->GetCollisionType() == ObjectCollisionType::Box)
		{
			return CheckCollision_BS(other, this);
		}
		else if (other->GetCollisionType() == ObjectCollisionType::Sphere)
		{
			return CheckCollision_SS(other);
		}
	}
	return false;
}

bool GameObject::CheckCollision_BB(GameObject* other)
{
	// AABB collision detection
	// @see vertex definition in Renderer2D.cpp
	bool bXAxis = GetPosition().x + GetScale().x / 2 >= other->GetPosition().x &&
		other->GetPosition().x + other->GetScale().x / 2 >= GetPosition().x;
	bool yAxis = GetPosition().y + GetScale().y / 2 >= other->GetPosition().y &&
		other->GetPosition().y + other->GetScale().y / 2 >= GetPosition().y;
	return bXAxis && yAxis;
}

bool GameObject::CheckCollision_BS(GameObject* boxObject, GameObject* sphereObject)
{
	// AABB circle collision detection
	BoxCollisionData* bcd = dynamic_cast<BoxCollisionData*>(boxObject->m_CollisionData);
	SphereCollisionData* scd = dynamic_cast<SphereCollisionData*>(sphereObject->m_CollisionData);
	float sphereRadius = scd->radius;
	glm::vec2 sphereCenter = scd->center;
	glm::vec2 boxExtents = bcd->extents;
	glm::vec2 boxCenter = bcd->center;
	glm::vec2 centerDiff = sphereCenter - boxCenter;
	glm::vec2 clampedDiff = glm::clamp(centerDiff, -boxExtents, boxExtents);
	glm::vec2 closestPosOnBox = boxCenter + clampedDiff;
	glm::vec2 closestDiff = closestPosOnBox - sphereCenter;
	return glm::length(closestDiff) <= sphereRadius;
}

bool GameObject::CheckCollision_SS(GameObject* other)
{
	// Circle-circle collision detection
	SphereCollisionData* scd = dynamic_cast<SphereCollisionData*>(m_CollisionData);
	SphereCollisionData* otherScd = dynamic_cast<SphereCollisionData*>(other->m_CollisionData);
	float radius = scd->radius;
	float otherRadius = otherScd->radius;
	glm::vec2 center = scd->center;
	glm::vec2 otherCenter = otherScd->center;
	float deltaX = center.x - otherCenter.x;
	float deltaY = center.y - otherCenter.y;
	return deltaX * deltaX + deltaY * deltaY <= (radius + otherRadius) * (radius + otherRadius);
}

void GameObject::ApplyDamage(GameObject* target, float damage)
{
	if (target)
	{
		target->TakeDamage(this, damage);
	}
}
