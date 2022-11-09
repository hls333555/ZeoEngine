#pragma once

#include <entt.hpp>

#include "Engine/GameFramework/FieldInstanceBase.h"

#include "Engine/Core/UUID.h"
#include "Engine/Utils/SceneUtils.h"

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

		const char* GetFieldName() const override;
		U32 GetFieldID() const override { return entt::hashed_string(GetFieldName()); }
		const char* GetFieldTooltip() const override { return nullptr; } // TODO:
		float GetDragSpeed() const override { return 1.0f; } // TODO:
		bool IsClampOnlyDuringDragging() const override { return false; } // TODO:
		virtual AssetTypeID GetAssetTypeID() const override { return 0; } // TODO:

		template<typename Type>
		Type GetDragMin() const // TODO:
		{
			return Type();
		}
		template<typename Type>
		Type GetDragMax() const // TODO:
		{
			return Type();
		}

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
			if (SceneUtils::IsLevelRuntime())
			{
				std::string value;
				GetRuntimeValueInternal(value);
				return value;
			}
			return *reinterpret_cast<std::string*>(Buffer);
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
				(*reinterpret_cast<std::string*>(Buffer)).assign(value);
			}
		}

		virtual void* GetValueRaw() const override;
		virtual void SetValueRaw(const void* value) const override;

		void CopyValueFromRuntime();
		void CopyValueToRuntime() const;

	private:
		void GetValue_Internal(void* outValue) const;
		void SetValue_Internal(const void* value) const;
		void GetRuntimeValueInternal(void* outValue) const;
		void GetRuntimeValueInternal(std::string& outValue) const;
		//void GetRuntimeValueInternal(AssetHandle* outValue) const;
		void SetRuntimeValueInternal(const void* value) const;
		void SetRuntimeValueInternal(const std::string& value) const;
		//void SetRuntimeValueInternal(const AssetHandle* value) const;

	private:
		ScriptField* Field = nullptr;
		UUID EntityID;

		U8* Buffer = nullptr;
		/** Buffer dedicated for retrieving runtime raw value */
		U8* RuntimeBuffer = nullptr;
	};
	
}
