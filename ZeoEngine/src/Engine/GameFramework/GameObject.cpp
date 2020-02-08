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
			metadata(PropertyMeta::Tooltip, u8"控制物体的移动，可渲染的Z坐标范围(-0.99, 0.99]"),
			metadata(PropertyMeta::DragSensitivity, 0.1f)
		)
		.property("Rotation", &Transform::rotation)
		(
			metadata(PropertyMeta::Tooltip, u8"控制物体的旋转。"),
			metadata(PropertyMeta::Min, -180.0f),
			metadata(PropertyMeta::Max, 180.0f)
		)
		.property("Scale", &Transform::scale)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Tooltip, u8"控制物体的缩放。"),
			// TODO: Currently negative scale values are not supported as ImGuizmo clamps that to 0.01
			metadata(PropertyMeta::Min, 0.01f),
			metadata(PropertyMeta::DragSensitivity, 0.1f)
		);

	registration::class_<CollisionData>("CollisionData")
		.property("CenterOffset", &CollisionData::CenterOffset)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Tooltip, u8"碰撞体的中心较物体中心的偏移。"),
			metadata(PropertyMeta::DragSensitivity, 0.1f)
		);

	registration::class_<BoxCollisionData>("BoxCollisionData")
		.property("Extents", &BoxCollisionData::Extents)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Tooltip, u8"盒子碰撞体的延展"),
			metadata(PropertyMeta::Min, 0.01f),
			metadata(PropertyMeta::DragSensitivity, 0.1f)
		);

	registration::class_<SphereCollisionData>("SphereCollisionData")
		.property("Radius", &SphereCollisionData::Radius)
		(
			metadata(PropertyMeta::Tooltip, u8"球形碰撞体的半径"),
			metadata(PropertyMeta::Min, 0.01f),
			metadata(PropertyMeta::DragSensitivity, 0.1f)
		);

	registration::class_<GameObject>("GameObject")
		.enumeration<ObjectCollisionType>("ObjectCollisionType")
		(
			value("None", ObjectCollisionType::None),
			value("Box Collision", ObjectCollisionType::Box),
			value("Sphere Collision", ObjectCollisionType::Sphere)
		)
		.method("OnPropertyValueChange", &GameObject::OnPropertyValueChange)
		.property("Name", &GameObject::m_Name)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Tooltip, u8"用于在编辑器里显示的名称。设为独一无二的最佳。")
		)
		.property("IsActive", &GameObject::m_bIsActive)
		.property("Transform", &GameObject::m_Transform)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Tooltip, u8"Transform组件，用于控制基本的移动，旋转和缩放。")
		)
		.property("CollisionType", &GameObject::m_CollisionType)
		(
			metadata(PropertyMeta::Category, "Collision"),
			metadata(PropertyMeta::Tooltip, u8"碰撞体类型，设为None来禁用碰撞。")
		)
		.property("CollisionData", &GameObject::m_CollisionData)
		(
			metadata(PropertyMeta::Category, "Collision"),
			metadata(PropertyMeta::Tooltip, u8"碰撞体配置")
		)
		.property("GenerateOverlapEvents", &GameObject::m_bGenerateOverlapEvents)
		(
			metadata(PropertyMeta::Category, "Collision"),
			metadata(PropertyMeta::Tooltip, u8"是否产生碰撞事件")
		)
		.property("Speed", &GameObject::m_Speed)
		(
			metadata(PropertyMeta::Category, "Movement"),
			metadata(PropertyMeta::Min, 0.0f),
			metadata(PropertyMeta::Max, 20.0f),
			metadata(PropertyMeta::DragSensitivity, 0.5f)
		);
}

namespace ZeoEngine {

	void BoxCollisionData::UpdateData()
	{
		if (OwnerObject)
		{
			Center = OwnerObject->GetPosition2D() + CenterOffset;
			if (Extents.x <= 0.0f || Extents.y <= 0.0f)
			{
				// By default, use object's bounds as extents
				Extents = OwnerObject->GetScale() / 2.0f;
			}
		}
	}

	void SphereCollisionData::UpdateData()
	{
		if (OwnerObject)
		{
			Center = OwnerObject->GetPosition2D() + CenterOffset;
			if (Radius <= 0.0f)
			{
				// By default, use object's half X scale as radius
				Radius = OwnerObject->GetScale().x / 2.0f;
			}
		}
	}

	GameObject::~GameObject()
	{
		delete m_CollisionData;
	}

	void GameObject::Init()
	{
		ReComposeTransformMatrix();
		GenerateCollisionData();
	}

	void GameObject::OnUpdate(DeltaTime dt)
	{
		// Moving to target position
		{
			if (m_bIsMoving)
			{
				m_MoveAlpha += GetSpeed() / m_MoveDistance * dt;
				SetPosition2D(glm::lerp(m_MoveSourcePosition, m_MoveTargetPosition, m_MoveAlpha));
				if (m_MoveAlpha >= 1.0f)
				{
					m_bIsMoving = false;
				}
			}
			else
			{
				m_MoveAlpha = 0.0f;
			}
		}

		m_Velocity = (GetPosition2D() - m_LastPosition) / (float)dt;
		m_LastPosition = GetPosition2D();
	}

	void GameObject::OnPropertyValueChange(const rttr::property& prop)
	{
		if (prop.get_name() == "CollisionType")
		{
			GenerateCollisionData();
		}
	}

	const glm::vec2 GameObject::GetForwardVector2D() const
	{
		return { sin(glm::radians(m_Transform.rotation)), cos(glm::radians(m_Transform.rotation)) };
	}

	const glm::vec2 GameObject::GetRightVector2D() const
	{
		const glm::vec3 zVector{ 0.0f, 0.0f, -1.0f };
		const glm::vec3 forwardVector(sin(glm::radians(m_Transform.rotation)), cos(glm::radians(m_Transform.rotation)), 0.0f);
		const glm::vec3 rightVector = glm::cross(zVector, forwardVector);
		return { rightVector.x, rightVector.y };
	}

	float GameObject::FindLookAtRotation2D(const glm::vec2& sourcePosition, const glm::vec2& targetPosition)
	{
		float deltaX = targetPosition.x - sourcePosition.x;
		float deltaY = targetPosition.y - sourcePosition.y;
		return atanf(deltaX / deltaY);
	}

	void GameObject::TranslateTo2D(const glm::vec2& targetPosition)
	{
		if (!m_bIsMoving)
		{
			m_MoveSourcePosition = GetPosition2D();
			m_MoveTargetPosition = targetPosition;
			m_MoveDistance = glm::length(m_MoveSourcePosition - m_MoveTargetPosition);
			m_bIsMoving = true;
		}
	}

	glm::vec2 GameObject::GetRandomPositionInRange2D(const glm::vec2& center, const glm::vec2& extents)
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

	void GameObject::GenerateCollisionData()
	{
		delete m_CollisionData;
		m_CollisionData = nullptr;
		if (m_CollisionType == ObjectCollisionType::Box)
		{
			m_CollisionData = new BoxCollisionData(this);
		}
		else if (m_CollisionType == ObjectCollisionType::Sphere)
		{
			m_CollisionData = new SphereCollisionData(this);
		}
	}

	void GameObject::FillBoxCollisionData(const glm::vec2& extents, const glm::vec2& centerOffset)
	{
		BoxCollisionData* boxCollision = dynamic_cast<BoxCollisionData*>(m_CollisionData);
		ZE_CORE_ASSERT_INFO(boxCollision, "Collision data has not been generated or it is not a box collision data!");
		boxCollision->CenterOffset = centerOffset;
		boxCollision->Extents = extents;
		boxCollision->UpdateData();
	}

	void GameObject::FillSphereCollisionData(float radius, const glm::vec2& centerOffset)
	{
		SphereCollisionData* sphereCollision = dynamic_cast<SphereCollisionData*>(m_CollisionData);
		ZE_CORE_ASSERT_INFO(sphereCollision, "Collision data has not been generated or it is not a sphere collision data!");
		sphereCollision->CenterOffset = centerOffset;
		sphereCollision->Radius = radius;
		sphereCollision->UpdateData();
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
		float sphereRadius = scd->Radius;
		glm::vec2 sphereCenter = scd->Center;
		glm::vec2 boxExtents = bcd->Extents;
		glm::vec2 boxCenter = bcd->Center;
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
		float radius = scd->Radius;
		float otherRadius = otherScd->Radius;
		glm::vec2 center = scd->Center;
		glm::vec2 otherCenter = otherScd->Center;
		float deltaX = center.x - otherCenter.x;
		float deltaY = center.y - otherCenter.y;
		return deltaX * deltaX + deltaY * deltaY <= (radius + otherRadius) * (radius + otherRadius);
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

	void GameObject::ApplyDamage(float damage, GameObject* target, GameObject* causer, GameObject* instigator)
	{
		if (target)
		{
			target->TakeDamage(damage, causer, instigator);
		}
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

}
