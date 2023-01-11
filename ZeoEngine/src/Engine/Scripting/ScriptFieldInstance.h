#pragma once

#include <entt.hpp>

#include "Engine/GameFramework/FieldInstanceBase.h"

#include "Engine/Core/UUID.h"
#include "Engine/Utils/SceneUtils.h"
#include "Engine/Core/Buffer.h"

namespace ZeoEngine {

	struct ScriptField;

	class ScriptFieldInstance : public FieldInstanceBase
	{
		friend class ScriptEngine;

	public:
		ScriptFieldInstance(ScriptField* field, UUID entityID);
		ScriptFieldInstance(const ScriptFieldInstance& other);
		ScriptFieldInstance(ScriptFieldInstance&& other) noexcept;
		~ScriptFieldInstance();

		ScriptFieldInstance& operator=(const ScriptFieldInstance& other);
		ScriptFieldInstance& operator=(ScriptFieldInstance&& other) noexcept;

		virtual const char* GetFieldName() const override;
		virtual U32 GetFieldID() const override { return entt::hashed_string(GetFieldName()); }
		virtual std::string GetFieldTooltip() const override;
		virtual float GetDragSpeed() const override;
		virtual bool IsClampOnlyDuringDragging() const override;
		virtual AssetTypeID GetAssetTypeID() const override;
		template<typename Type>
		Type GetDragMin() const
		{
			return m_Field->GetAttributeValue<Type>("ClampMin").value_or(EngineUtils::GetDefaultMin<Type>());
		}
		template<typename Type>
		Type GetDragMax() const
		{
			return m_Field->GetAttributeValue<Type>("ClampMax").value_or(EngineUtils::GetDefaultMax<Type>());
		}

		bool IsHidden() const;
		bool IsTransient() const;
		std::string GetCategory() const;

		template<typename T>
		T GetValue() const
		{
			T value;
			if (SceneUtils::IsLevelRuntime())
			{
				GetRuntimeValueInternal(&value);
			}
			else
			{
				GetValue_Internal(&value);
			}
			return value;
		}

		template<>
		std::string GetValue() const
		{
			std::string value;
			if (SceneUtils::IsLevelRuntime())
			{
				GetRuntimeValueInternal(value);
			}
			else
			{
				GetValue_Internal(value);
			}
			return value;
		}

		template<typename T>
		void SetValue(T&& value)
		{
			if (SceneUtils::IsLevelRuntime())
			{
				SetRuntimeValueInternal(&value);
			}
			else
			{
				SetValue_Internal(&value);
			}
		}

		template<>
		void SetValue(const std::string& value)
		{
			if (SceneUtils::IsLevelRuntime())
			{
				SetRuntimeValueInternal(value);
			}
			else
			{
				SetValue_Internal(value);
			}
		}

		virtual void* GetValueRaw() const override;
		virtual void SetValueRaw(const void* value) const override;

		void CopyValueFromRuntime();
		void CopyValueToRuntime() const;

	private:
		void GetValue_Internal(void* outValue) const;
		void GetValue_Internal(std::string& outValue) const;
		void SetValue_Internal(const void* value) const;
		void SetValue_Internal(const std::string& value) const;
		void GetRuntimeValueInternal(void* outValue) const;
		void GetRuntimeValueInternal(std::string& outValue) const;
		void SetRuntimeValueInternal(const void* value) const;
		void SetRuntimeValueInternal(const std::string& value) const;

	private:
		ScriptField* m_Field = nullptr;
		UUID m_EntityID;

		Buffer m_Buffer;
		/** Buffer dedicated for retrieving runtime raw value */
		Buffer m_RuntimeBuffer;
	};
	
}
