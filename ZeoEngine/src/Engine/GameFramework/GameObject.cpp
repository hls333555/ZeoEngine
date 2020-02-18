#include "ZEpch.h"
#include "Engine/GameFramework/GameObject.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
#define RAPIDJSON_HAS_STDSTRING 1
#include <prettywriter.h> // for stringify JSON
#include <document.h>     // rapidjson's DOM-style API

#include "Engine/Core/EngineGlobals.h"
#include "Engine/GameFramework/Level.h"
#include "Engine/Core/RandomEngine.h"
#include "Engine/Layers/EditorLayer.h"
#include "Engine/Core/EngineUtilities.h"

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
		.property("DrawCollision", &CollisionData::bDrawCollision)
		(
			metadata(PropertyMeta::Tooltip, u8"是否绘制碰撞体。")
		)
		.property("CenterOffset", &CollisionData::CenterOffset)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Tooltip, u8"碰撞体的中心较物体中心的偏移。"),
			metadata(PropertyMeta::DragSensitivity, 0.05f)
		);
	registration::class_<BoxCollisionData>("BoxCollisionData")
		.property("Extents", &BoxCollisionData::Extents)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Tooltip, u8"盒子碰撞体的延展"),
			metadata(PropertyMeta::Min, 0.01f),
			metadata(PropertyMeta::DragSensitivity, 0.05f)
		);
	registration::class_<SphereCollisionData>("SphereCollisionData")
		.property("Radius", &SphereCollisionData::Radius)
		(
			metadata(PropertyMeta::Tooltip, u8"球形碰撞体的半径"),
			metadata(PropertyMeta::Min, 0.01f),
			metadata(PropertyMeta::DragSensitivity, 0.05f)
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
			metadata(PropertyMeta::Tooltip, u8"用于在编辑器里显示的名称。")
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
		RecomposeTransformMatrix();
		GenerateCollisionData();
	}

	void GameObject::BeginPlay()
	{
		m_bHasBegunPlay = true;
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

	void GameObject::OnGameViewImGuiRender()
	{
		// Draw collision
		if (m_CollisionData && m_CollisionData->bDrawCollision)
		{
			// Do not draw in PIE mode
			if (g_PIEState != PIEState::None)
				return;

			EditorLayer* editor = Application::Get().FindLayerByName<EditorLayer>("Editor");
			ImDrawList* dl = ImGui::GetWindowDrawList();
			const glm::vec2 collisionScreenCenter = ZeoEngine::ProjectWorldToScreen2D(GetPosition2D() + m_CollisionData->CenterOffset, ImGui::GetCurrentWindow(), editor->GetEditorCamera());
			static const ImU32 collisionColor = IM_COL32(255, 136, 0, 255); // Orange color
			static const float collisionThickness = 2.5f;
			if (m_CollisionType == ObjectCollisionType::Box)
			{
				const float collisionScreenExtentX = dynamic_cast<BoxCollisionData*>(m_CollisionData)->Extents.x / editor->GetEditorCamera()->GetCameraBounds().Right * ImGui::GetCurrentWindow()->InnerRect.GetSize().x / 2;
				const float collisionScreenExtentY = dynamic_cast<BoxCollisionData*>(m_CollisionData)->Extents.y / editor->GetEditorCamera()->GetCameraBounds().Top * ImGui::GetCurrentWindow()->InnerRect.GetSize().y / 2;
				dl->AddRect(ImVec2(collisionScreenCenter.x - collisionScreenExtentX, collisionScreenCenter.y - collisionScreenExtentY),
					ImVec2(collisionScreenCenter.x + collisionScreenExtentX, collisionScreenCenter.y + collisionScreenExtentY), collisionColor,
					0.0f, 15, collisionThickness);
			}
			else if (m_CollisionType == ObjectCollisionType::Sphere)
			{
				const float collisionScreenRadius = dynamic_cast<SphereCollisionData*>(m_CollisionData)->Radius / editor->GetEditorCamera()->GetCameraBounds().Right * ImGui::GetCurrentWindow()->InnerRect.GetSize().x / 2;
				dl->AddCircle(ImVec2(collisionScreenCenter.x, collisionScreenCenter.y), collisionScreenRadius, collisionColor, 36, collisionThickness);
			}
		}
	}

	void GameObject::OnPropertyValueChange(const rttr::property& prop)
	{
		if (prop.get_name() == "CollisionType")
		{
			GenerateCollisionData();
		}
	}

	void SerializeRecursively(const rttr::instance& object, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);

	// TODO: Save camera position
	std::string GameObject::Serialize()
	{
		rapidjson::StringBuffer sb;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);

		SerializeRecursively(*this, writer);
		return sb.GetString();
	}

	bool SerializeAtomicTypes(const rttr::type& type, const rttr::variant& var, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
	{
		if (type.is_arithmetic())
		{
			if (type == rttr::type::get<bool>())
			{
				writer.Bool(var.to_bool());
			}
			else if (type == rttr::type::get<char>())
			{
				writer.Bool(var.to_bool());
			}
			else if (type == rttr::type::get<int8_t>())
			{
				writer.Int(var.to_int8());
			}
			else if (type == rttr::type::get<int16_t>())
			{
				writer.Int(var.to_int16());
			}
			else if (type == rttr::type::get<int32_t>())
			{
				writer.Int(var.to_int32());
			}
			else if (type == rttr::type::get<int64_t>())
			{
				writer.Int64(var.to_int64());
			}
			else if (type == rttr::type::get<uint8_t>())
			{
				writer.Uint(var.to_uint8());
			}
			else if (type == rttr::type::get<uint16_t>())
			{
				writer.Uint(var.to_uint16());
			}
			else if (type == rttr::type::get<uint32_t>())
			{
				writer.Uint(var.to_uint32());
			}
			else if (type == rttr::type::get<uint64_t>())
			{
				writer.Uint64(var.to_uint64());
			}
			else if (type == rttr::type::get<float>())
			{
				writer.Double(var.to_double());
			}
			else if (type == rttr::type::get<double>())
			{
				writer.Double(var.to_double());
			}
			return true;
		}
		// enum
		else if (type.is_enumeration())
		{
			bool bOk = false;
			const std::string stringValue = var.to_string(&bOk);
			if (bOk)
			{
				writer.String(stringValue);
			}
			else
			{
				bOk = false;
				uint64_t uint64Value = var.to_uint64(&bOk);
				if (bOk)
				{
					writer.Uint64(uint64Value);
				}
				else
				{
					writer.Null();
				}
			}
			return true;
		}
		// std::string
		else if (type.get_raw_type() == rttr::type::get<std::string>())
		{
			if (type.is_pointer())
			{
				writer.String(*var.get_value<std::string*>());
			}
			else
			{
				writer.String(var.to_string());
			}
			return true;
		}
		// glm::vec2
		else if (type.get_raw_type() == rttr::type::get<glm::vec2>())
		{
			if (type.is_pointer())
			{
				writer.StartArray();
				writer.Double(var.get_value<glm::vec2*>()->x);
				writer.Double(var.get_value<glm::vec2*>()->y);
				writer.EndArray();
			}
			else
			{
				writer.StartArray();
				writer.Double(var.get_value<glm::vec2>().x);
				writer.Double(var.get_value<glm::vec2>().y);
				writer.EndArray();
			}
			return true;
		}
		// glm::vec3
		else if (type.get_raw_type() == rttr::type::get<glm::vec3>())
		{
			if (type.is_pointer())
			{
				writer.StartArray();
				writer.Double(var.get_value<glm::vec3*>()->x);
				writer.Double(var.get_value<glm::vec3*>()->y);
				writer.Double(var.get_value<glm::vec3*>()->z);
				writer.EndArray();
			}
			else
			{
				writer.StartArray();
				writer.Double(var.get_value<glm::vec3>().x);
				writer.Double(var.get_value<glm::vec3>().y);
				writer.Double(var.get_value<glm::vec3>().z);
				writer.EndArray();
			}
			return true;
		}
		// glm::vec4
		else if (type.get_raw_type() == rttr::type::get<glm::vec4>())
		{
			if (type.is_pointer())
			{
				writer.StartArray();
				writer.Double(var.get_value<glm::vec4*>()->x);
				writer.Double(var.get_value<glm::vec4*>()->y);
				writer.Double(var.get_value<glm::vec4*>()->z);
				writer.Double(var.get_value<glm::vec4*>()->w);
				writer.EndArray();
			}
			else
			{
				writer.StartArray();
				writer.Double(var.get_value<glm::vec4>().x);
				writer.Double(var.get_value<glm::vec4>().y);
				writer.Double(var.get_value<glm::vec4>().z);
				writer.Double(var.get_value<glm::vec4>().w);
				writer.EndArray();
			}
			return true;
		}
		// TODO: Serialize GameObject*
		// GameObject*
		else if (type.is_pointer() && type == rttr::type::get<GameObject*>())
		{
			writer.String(var.get_value<GameObject*>()->GetUniqueName());
			return true;                      
		}
		// Ref<Texture2D>
		else if (type.get_raw_type() == rttr::type::get<Texture2D>())
		{
			const auto& texture = var.get_value<Ref<Texture2D>>();
			writer.String(texture ? texture->GetPath() : "");
			return true;
		}
		return false;
	}

	void SerializeSequentialContainerTypes(const rttr::variant_sequential_view& sequentialView, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
	{
		writer.StartArray();
		for (const auto& item : sequentialView)
		{
			if (item.is_sequential_container())
			{
				SerializeSequentialContainerTypes(item.create_sequential_view(), writer);
			}
			else
			{
				rttr::variant wrappedVar = item.extract_wrapped_value();
				rttr::type valueType = wrappedVar.get_type();
				if (SerializeAtomicTypes(valueType, wrappedVar, writer))
				{
				}
				else
				{
					SerializeRecursively(wrappedVar, writer);
				}
			}
		}
		writer.EndArray();
	}

	bool SerializeValue(const rttr::variant& var, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer);

	void SerializeAssociativeContainerTypes(const rttr::variant_associative_view& associativeView, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
	{
		static const std::string keyName("key");
		static const std::string valueName("value");

		writer.StartArray();
		if (associativeView.is_key_only_type())
		{
			for (auto& item : associativeView)
			{
				SerializeValue(item.first, writer);
			}
		}
		else
		{
			for (auto& item : associativeView)
			{
				writer.StartObject();
				writer.String(keyName.c_str(), static_cast<rapidjson::SizeType>(keyName.size()), false);
				SerializeValue(item.first, writer);
				writer.String(valueName.c_str(), static_cast<rapidjson::SizeType>(valueName.size()), false);
				SerializeValue(item.second, writer);
				writer.EndObject();
			}
		}
		writer.EndArray();
	}

	bool SerializeValue(const rttr::variant& var, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
	{
		rttr::type valueType = var.get_type();
		rttr::type wrappedType = valueType.is_wrapper() ? valueType.get_wrapped_type() : valueType;
		bool is_wrapper = wrappedType != valueType;
		if (SerializeAtomicTypes(wrappedType, is_wrapper ? var.extract_wrapped_value() : var, writer))
		{
		}
		else if (var.is_sequential_container())
		{
			SerializeSequentialContainerTypes(var.create_sequential_view(), writer);
		}
		else if (var.is_associative_container())
		{
			SerializeAssociativeContainerTypes(var.create_associative_view(), writer);
		}
		else
		{
			auto& childProps = is_wrapper ? wrappedType.get_properties() : valueType.get_properties();
			if (!childProps.empty())
			{
				SerializeRecursively(var, writer);
			}
			else
			{
				bool bOk = false;
				std::string stringValue = var.to_string(&bOk);
				if (!bOk)
				{
					writer.String(stringValue);
					return false;
				}
				writer.String(stringValue);
			}
		}
		return true;
	}

	void SerializeRecursively(const rttr::instance& object, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer)
	{
		writer.StartObject();
		rttr::instance obj = object.get_type().get_raw_type().is_wrapper() ? object.get_wrapped_instance() : object;
		const auto& properties = obj.get_derived_type().get_properties();
		for (auto prop : properties)
		{
			if (prop.get_metadata(PropertyMeta::Transient))
				continue;

			rttr::variant var = prop.get_value(obj);
			// Cannot serialize, because we cannot retrieve the value
			if (!var)
				continue;

			const auto name = prop.get_name();
			writer.String(name.data(), static_cast<rapidjson::SizeType>(name.length()), false);
			if (!SerializeValue(var, writer))
			{
				ZE_CORE_ERROR("Failed to serialize property: {0}", name);
			}
		}
		writer.EndObject();
	}

	void GameObject::OnDeserialized()
	{
		RecomposeTransformMatrix();
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
		if (!m_bPendingDestroy)
		{
			m_bPendingDestroy = true;
			m_bIsActive = false;
			OnDestroyed();
			if (m_OnDestroyed)
			{
				m_OnDestroyed();
			}
			Level::Get().PendingDestroyGameObject(this);
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

	void GameObject::RecomposeTransformMatrix()
	{
		glm::vec3 rotation = glm::vec3(0.0f, 0.0f, m_Transform.rotation);
		glm::vec3 scale = glm::vec3(m_Transform.scale.x, m_Transform.scale.y, 0.0f);
		ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(m_Transform.position), glm::value_ptr(rotation), glm::value_ptr(scale), glm::value_ptr(m_TransformMatrix));
	}

}
