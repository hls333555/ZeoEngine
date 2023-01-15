#include "Inspectors/ComponentFieldInstance.h"

#include "Engine/Utils/EngineUtils.h"
#include "Engine/Utils/ReflectionUtils.h"

namespace ZeoEngine {

	ComponentFieldInstance::ComponentFieldInstance(FieldType type, entt::meta_data data, Entity entity, U32 compID)
		: FieldInstanceBase(type)
		, m_Data(data)
		, m_Entity(entity)
		, m_ComponentID(compID)
	{
	}

	const char* ComponentFieldInstance::GetFieldName() const
	{
		return ReflectionUtils::GetMetaObjectName(m_Data);
	}

	float ComponentFieldInstance::GetDragSpeed() const
	{
		return ReflectionUtils::GetPropertyValue<float>(Reflection::DragSensitivity, m_Data).value_or(1.0f);
	}

	bool ComponentFieldInstance::IsClampOnlyDuringDragging() const
	{
		return ReflectionUtils::DoesPropertyExist(Reflection::ClampOnlyDuringDragging, m_Data);
	}

	AssetTypeID ComponentFieldInstance::GetAssetTypeID() const
	{
		return ReflectionUtils::GetPropertyValue<AssetTypeID>(Reflection::AssetType, m_Data).value_or(0);
	}

	std::string ComponentFieldInstance::GetFieldTooltip() const
	{
		const auto tooltip = ReflectionUtils::GetPropertyValue<const char*>(Reflection::Tooltip, m_Data);
		return tooltip ? *tooltip : "";
	}

	void* ComponentFieldInstance::GetValueRaw()
	{
		return GetValueInternal().data();
	}

	void ComponentFieldInstance::SetValueRaw(const void* value)
	{
		// For this to work, the meta data must be registered by entt::as_ref_t
		memcpy(GetValueRaw(), value, GetFieldSize());
	}

	void ComponentFieldInstance::OnFieldValueChanged()
	{
		m_Entity.PatchComponentByID(m_ComponentID, GetFieldID());
	}

	entt::meta_any ComponentFieldInstance::GetValueInternal() const
	{
		return m_Data.get(m_Entity.GetComponentByID(m_ComponentID));
	}

	IComponent* ComponentFieldInstance::GetComponent() const
	{
		return m_Entity.GetComponentByID(m_ComponentID).try_cast<IComponent>();
	}

	entt::meta_type ComponentSequenceContainerElementFieldInstance::GetFieldValueType() const
	{
		return GetValueInternal().type();
	}

	entt::meta_any ComponentSequenceContainerElementFieldInstance::GetValueInternal() const
	{
		return ComponentFieldInstance::GetValueInternal().as_sequence_container()[m_Index];
	}

}
