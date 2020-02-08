#include "ZEpch.h"
#include "Engine/GameFramework/GameObject.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

#include "Engine/Core/EngineGlobals.h"
#include "Engine/GameFramework/Level.h"
#include "Engine/Core/RandomEngine.h"

RTTR_REGISTRATION
{
	using namespace rttr;
	using namespace ZeoEngine;

	registration::class_<Transform>("Transform")
		.property("Position", &Transform::position)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Tooltip, u8"����������ƶ�������Ⱦ��Z���귶Χ(-0.99, 0.99]"),
			metadata(PropertyMeta::DragSensitivity, 0.1f)
		)
		.property("Rotation", &Transform::rotation)
		(
			metadata(PropertyMeta::Tooltip, u8"�����������ת��"),
			metadata(PropertyMeta::Min, -180.0f),
			metadata(PropertyMeta::Max, 180.0f)
		)
		.property("Scale", &Transform::scale)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Tooltip, u8"������������š�"),
			// TODO: Currently negative scale values are not supported as ImGuizmo clamps that to 0.01
			metadata(PropertyMeta::Min, 0.01f),
			metadata(PropertyMeta::DragSensitivity, 0.1f)
		);

	registration::class_<GameObject>("GameObject")
		.property("Name", &GameObject::m_Name)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Tooltip, u8"�����ڱ༭������ʾ�����ơ���Ϊ��һ�޶�����ѡ�")
		)
		.property("IsActive", &GameObject::m_bIsActive)
		.property("Transform", &GameObject::m_Transform)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Tooltip, u8"Transform������������ƻ������ƶ�����ת�����š�")
		)
		.property("Speed", &GameObject::m_Speed)
		(
			metadata(PropertyMeta::Min, 0.0f),
			metadata(PropertyMeta::Max, 20.0f),
			metadata(PropertyMeta::DragSensitivity, 0.5f)
		);
}

namespace ZeoEngine {

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

	GameObject::~GameObject()
	{
		delete m_CollisionData;
	}

	void GameObject::Destroy()
	{
		if (!bPendingDestroy)
		{
			bPendingDestroy = true;
			m_bIsActive = false;
			OnDestroyed();
			Level::Get().DestroyGameObject(this);
		}

	}

	void GameObject::Init()
	{
		ReComposeTransformMatrix();
	}

	void GameObject::OnUpdate(DeltaTime dt)
	{
		// Moving to target position
		{
			if (m_bStartMoving)
			{
				m_MovingAlpha += GetSpeed() / m_MovingDistance * dt;
				SetPosition2D(glm::lerp(m_SourcePosition, m_TargetPosition, m_MovingAlpha));
				if (m_MovingAlpha >= 1.0f)
				{
					m_bStartMoving = false;
				}
			}
			else
			{
				m_MovingAlpha = 0.0f;
			}
		}

		m_Velocity = (GetPosition2D() - m_LastPosition) / (float)dt;
		m_LastPosition = GetPosition2D();
	}

	const glm::vec2 GameObject::GetForwardVector() const
	{
		return { sin(glm::radians(m_Transform.rotation)), cos(glm::radians(m_Transform.rotation)) };
	}

	const glm::vec2 GameObject::GetRightVector() const
	{
		const glm::vec3 zVector({ 0.0f, 0.0f, -1.0f });
		const glm::vec3 forwardVector(sin(glm::radians(m_Transform.rotation)), cos(glm::radians(m_Transform.rotation)), 0.0f);
		const glm::vec3 rightVector = glm::cross(zVector, forwardVector);
		return { rightVector.x, rightVector.y };
	}

	float GameObject::FindLookAtRotation(const glm::vec2& sourcePosition, const glm::vec2& targetPosition)
	{
		float deltaX = targetPosition.x - sourcePosition.x;
		float deltaY = targetPosition.y - sourcePosition.y;
		return atanf(deltaX / deltaY);
	}

	void GameObject::TranslateTo(const glm::vec2& targetPosition)
	{
		if (!m_bStartMoving)
		{
			m_SourcePosition = GetPosition2D();
			m_TargetPosition = targetPosition;
			m_MovingDistance = glm::length(m_SourcePosition - m_TargetPosition);
			m_bStartMoving = true;
		}
	}

	glm::vec2 GameObject::GetRandomPositionInRange(const glm::vec2& center, const glm::vec2& extents)
	{
		// TODO: Random position is limited by camera bounds
		float lowerX = std::max(center.x - extents.x, ZeoEngine::GetActiveGameCamera()->GetCameraBounds().Left + 0.5f);
		float upperX = std::min(center.x + extents.x, ZeoEngine::GetActiveGameCamera()->GetCameraBounds().Right - 0.5f);
		float lowerY = std::max(center.y - extents.y, ZeoEngine::GetActiveGameCamera()->GetCameraBounds().Bottom + 0.5f);
		float upperY = std::min(center.y + extents.y, ZeoEngine::GetActiveGameCamera()->GetCameraBounds().Top - 0.5f);
		float randomX = RandomEngine::RandFloatInRange(lowerX, upperX);
		float randomY = RandomEngine::RandFloatInRange(lowerY, upperY);
		return { randomX, randomY };
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
			if (!m_bIsActive)
				return;

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

	void GameObject::DecomposeTransformMatrix()
	{
		float translation[3], rotation[3], scale[3];
		ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(m_TransformMatrix), translation, rotation, scale);
		// For 2D rendering, we do not want ZPos to be modified via gizmo editing
		m_Transform.position = glm::vec3(translation[0], translation[1], m_Transform.position.z);
		m_Transform.rotation = rotation[2];
		m_Transform.scale = glm::vec2(scale[0], scale[1]);
	}

	void GameObject::ReComposeTransformMatrix()
	{
		glm::vec3 rotation = glm::vec3(0.0f, 0.0f, m_Transform.rotation);
		glm::vec3 scale = glm::vec3(m_Transform.scale.x, m_Transform.scale.y, 0.0f);
		ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(m_Transform.position), glm::value_ptr(rotation), glm::value_ptr(scale), glm::value_ptr(m_TransformMatrix));
	}

	void GameObject::ApplyDamage(float damage, GameObject* target, GameObject* causer, GameObject* instigator)
	{
		if (target)
		{
			target->TakeDamage(damage, causer, instigator);
		}
	}

}