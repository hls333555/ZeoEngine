#pragma once

#include <entt.hpp>

#include "Engine/GameFramework/FieldInstanceBase.h"

#include "Engine/Core/UUID.h"
#include "Engine/Utils/SceneUtils.h"
#include "Engine/Core/Buffer.h"
#include "Engine/Scripting/ScriptField.h"

extern "C" {
	typedef struct _MonoArray MonoArray;
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
}

namespace ZeoEngine {

	struct ScriptField;

	class ScriptFieldInstance : public FieldInstanceBase
	{
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
		virtual bool IsFieldDisabled() const override { return false; }

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
		T GetValue()
		{
			T value;
			if (SceneUtils::IsLevelRuntime())
			{
				GetRuntimeValueInternal(&value);
			}
			else
			{
				GetValueInternal(&value);
			}
			return value;
		}

		template<>
		std::string GetValue()
		{
			std::string value;
			if (SceneUtils::IsLevelRuntime())
			{
				GetRuntimeValueInternal(value);
			}
			else
			{
				GetValueInternal(value);
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
				SetValueInternal(&value);
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
				SetValueInternal(value);
			}
		}

		virtual void* GetValueRaw() override;
		virtual void SetValueRaw(const void* value) override;
		virtual void OnFieldValueChanged() override;

		virtual void CopyValueFromRuntime();
		virtual void CopyValueToRuntime();

		virtual void OnReload(ScriptField* field) { m_Field = field; }

	protected:
		virtual void GetValueInternal(void* outValue) const;
		virtual void GetValueInternal(std::string& outValue) const;
		virtual void SetValueInternal(const void* value) const;
		virtual void SetValueInternal(const std::string& value) const;
		virtual void GetRuntimeValueInternal(void* outValue) const;
		virtual void GetRuntimeValueInternal(std::string& outValue) const;
		virtual void SetRuntimeValueInternal(const void* value) const;
		virtual void SetRuntimeValueInternal(const std::string& value) const;

	protected:
		Buffer m_Buffer;
		/** Allocated buffer dedicated for retrieving runtime raw value */
		Buffer m_RuntimeBuffer;
		bool m_bIsBufferAllocated = false;

		ScriptField* m_Field = nullptr;
		UUID m_EntityID;
	};

	class ScriptSequenceContainerFieldInstance : public ScriptFieldInstance
	{
	public:
		using ScriptFieldInstance::ScriptFieldInstance;

		SizeT GetContainerSize() const;
		FieldType GetElementType() const { return m_ElementType; }
		U32 GetElementSize() const { return EngineUtils::GetFieldSize(m_ElementType); }
		U32 GetIndex() const { return m_CurrentElementIndex; }
		void SetIndex(U32 index) { m_CurrentElementIndex = index; }

		bool InsertDefault(U32 index, bool bNeedsUpdateArray);
		bool Erase(U32 index);
		void Clear();
		void ResizeWithDefault(SizeT size);
		void MoveElement(U32 from, U32 to);

		virtual void* GetValueRaw() override;

		virtual void CopyValueFromRuntime() override;
		virtual void CopyValueToRuntime() override;

		virtual void OnReload(ScriptField* field) override;

		void GetRuntimeArrayData();

	private:
		virtual void GetValueInternal(void* outValue) const override;
		virtual void GetValueInternal(std::string& outValue) const override;
		virtual void SetValueInternal(const void* value) const override;
		virtual void SetValueInternal(const std::string& value) const override;
		virtual void GetRuntimeValueInternal(void* outValue) const override;
		virtual void GetRuntimeValueInternal(std::string& outValue) const override;
		virtual void SetRuntimeValueInternal(const void* value) const override;
		virtual void SetRuntimeValueInternal(const std::string& value) const override;

		template<typename SetValueFunc>
		void SetRuntimeArrayData(SetValueFunc func) const
		{
			if (auto* listObject = GetListObject())
			{
				auto* monoArray = GetListArray();
				ZE_CORE_ASSERT(monoArray);

				func(monoArray);

				// Clear the list
				auto* listClass = mono_object_get_class(listObject);
				auto* clearMethod = mono_class_get_method_from_name(listClass, "Clear", 0);
				MonoObject* exception = nullptr;
				mono_runtime_invoke(clearMethod, listObject, nullptr, &exception);

				// Add modified array to the list
				auto* addRangeMethod = mono_class_get_method_from_name(listClass, "AddRange", 1);
				void* params = monoArray; // NOTE how MonoArray* is passed as parameter
				mono_runtime_invoke(addRangeMethod, listObject, &params, &exception);
			}
		}

		MonoObject* GetListObject() const;
		MonoArray* GetListArray() const;

	private:
		/** Container size only used outside runtime, see GetContainerSize() **/
		SizeT m_ContainerSize = 0;
		MonoClass* m_ElementClass = nullptr;
		FieldType m_ElementType = FieldType::None;

		U32 m_CurrentElementIndex = 0;
		U32 m_ArrayHandle = 0;
	};
	
}
