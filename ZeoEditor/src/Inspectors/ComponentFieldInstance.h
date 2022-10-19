#pragma once

#include "Engine/GameFramework/FieldInstanceBase.h"

#include <entt.hpp>

#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	struct IComponent;

	class ComponentFieldInstance : public FieldInstanceBase
	{
	public:
		ComponentFieldInstance(FieldType type, entt::meta_data data, Entity entity, U32 compID);

		virtual const char* GetFieldName() const override;
		virtual U32 GetFieldID() const override { return m_Data.id(); }
		virtual const char* GetFieldTooltip() const override;
		virtual float GetDragSpeed() const override;
		virtual bool IsClampOnlyDuringDragging() const override;
		virtual AssetTypeID GetAssetTypeID() const override;

		virtual void* GetValueRaw() const override;
		virtual void SetValueRaw(const void* value) const override;

		entt::meta_data GetFieldData() const { return m_Data; }
		virtual entt::meta_type GetFieldValueType() const { return m_Data.type(); }
		Entity GetEntity() const { return m_Entity; }
		U32 GetComponentID() const { return m_ComponentID; }
		IComponent* GetComponent() const { return m_Entity.GetComponentByID(m_ComponentID).try_cast<IComponent>(); }
		template<typename Type>
		Type GetDragMin() const
		{
			return ReflectionUtils::GetPropertyValue<Type>(Reflection::ClampMin, m_Data).value_or(EngineUtils::GetDefaultMin<Type>());
		}
		template<typename Type>
		Type GetDragMax() const
		{
			return ReflectionUtils::GetPropertyValue<Type>(Reflection::ClampMax, m_Data).value_or(EngineUtils::GetDefaultMax<Type>());
		}

		template<typename T>
		T GetValue() const
		{
			auto v = GetValueInternal();
			if constexpr (std::is_same_v<T, bool>)
			{
				if (!v.try_cast<bool>())
				{
					return v.cast<std::vector<bool>::reference>();
				}
			}
			return v.cast<T>();
		}

		template<typename T>
		void SetValue(T&& value)
		{
			auto v = GetValueInternal();
			if constexpr (std::is_same_v<T, bool&>)
			{
				if (!v.try_cast<bool>())
				{
					v.cast<std::vector<bool>::reference&>() = value;
					return;
				}
			}
			v.cast<std::decay_t<T>&>() = value;
		}

		void OnFieldValueChanged(U32 fieldID);

		virtual entt::meta_any GetValueInternal() const;

	private:
		entt::meta_data m_Data;
		Entity m_Entity;
		U32 m_ComponentID;
	};

	class ComponentSequenceContainerElementFieldInstance : public ComponentFieldInstance
	{
	public:
		using ComponentFieldInstance::ComponentFieldInstance;

		U32 GetIndex() const { return m_Index; }
		void SetIndex(U32 index) { m_Index = index; }

		virtual entt::meta_type GetFieldValueType() const override;
		virtual entt::meta_any GetValueInternal() const override;

	private:
		U32 m_Index = 0;
	};
	
}
