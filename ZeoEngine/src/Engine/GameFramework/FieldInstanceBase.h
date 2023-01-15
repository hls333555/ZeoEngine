#pragma once

#include "Engine/Asset/Asset.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	class FieldInstanceBase
	{
	public:
		FieldInstanceBase(FieldType type)
			: m_Type(type) {}
		FieldInstanceBase(const FieldInstanceBase&) = default;
		FieldInstanceBase(FieldInstanceBase&&) noexcept = default;
		virtual ~FieldInstanceBase() = default;

		FieldInstanceBase& operator=(const FieldInstanceBase&) = default;
		FieldInstanceBase& operator=(FieldInstanceBase&&) noexcept = default;

		FieldType GetFieldType() const { return m_Type; }
		U32 GetFieldSize() const { return EngineUtils::GetFieldSize(m_Type); }

		virtual const char* GetFieldName() const = 0;
		virtual U32 GetFieldID() const = 0;

		virtual std::string GetFieldTooltip() const = 0;
		virtual float GetDragSpeed() const = 0;
		virtual bool IsClampOnlyDuringDragging() const = 0;
		virtual AssetTypeID GetAssetTypeID() const = 0;

		virtual void* GetValueRaw() = 0;
		virtual void SetValueRaw(const void* value) = 0;
		virtual void OnFieldValueChanged() = 0;

	private:
		FieldType m_Type = FieldType::None;
	};
	
}
