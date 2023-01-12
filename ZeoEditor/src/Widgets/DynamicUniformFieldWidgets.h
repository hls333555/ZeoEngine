#pragma once

#include "Widgets/AssetBrowser.h"
#include "Engine/Renderer/Material.h"

namespace ZeoEngine {

	class DynamicUniformFieldWidgetBase
	{
	public:
		virtual ~DynamicUniformFieldWidgetBase() = default;

		virtual void Draw() = 0;
	};

	class DynamicUniformBoolFieldWidget : public DynamicUniformFieldWidgetBase
	{
	public:
		DynamicUniformBoolFieldWidget(DynamicUniformBoolFieldBase* field);

		virtual void Draw() override;

	private:
		DynamicUniformBoolFieldBase* m_Field;
	};

	template<typename Type, U32 N = 1, typename BaseType = Type>
	class DynamicUniformScalarNFieldWidget : public DynamicUniformFieldWidgetBase
	{
	public:
		DynamicUniformScalarNFieldWidget(DynamicUniformScalarNField<Type, N, BaseType>* field)
			: m_Field(field)
		{
		}

		virtual void Draw() override
		{
			const auto min = EngineUtils::GetDefaultMin<BaseType>();
			const auto max = EngineUtils::GetDefaultMax<BaseType>();
			void* buffer = nullptr;
			if constexpr (N == 1)
			{
				buffer = m_Field->bIsEditActive ? &m_Field->Buffer : &m_Field->Value;
			}
			else
			{
				buffer = m_Field->bIsEditActive ? glm::value_ptr(m_Field->Buffer) : glm::value_ptr(m_Field->Value);
			}
			bool bChanged = ImGui::DragScalarNEx("", ShaderReflectionFieldTypeToImGuiDataType(), buffer, N, 0.5f, &min, &max, GetOutputFormatByShaderReflectionFieldType(), ImGuiSliderFlags_AlwaysClamp);
			if (bChanged && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				m_Field->Apply();
			}
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				m_Field->bIsEditActive = false;
				if (m_Field->Value != m_Field->Buffer)
				{
					m_Field->Apply();
				}
			}
			if (ImGui::IsItemActivated())
			{
				m_Field->bIsEditActive = true;
				m_Field->Buffer = m_Field->Value;
			}
		}

	private:
		ImGuiDataType ShaderReflectionFieldTypeToImGuiDataType() const
		{
			if constexpr (std::is_same_v<Type, I32>)
			{
				return ImGuiDataType_S32;
			}
			else
			{
				return ImGuiDataType_Float;
			}
		}

		const char* GetOutputFormatByShaderReflectionFieldType() const
		{
			if constexpr (std::is_same_v<Type, I32>)
			{
				return "%d";
			}
			else
			{
				return "%.3f";
			}
		}

	private:
		DynamicUniformScalarNField<Type, N, BaseType>* m_Field;
	};

	class DynamicUniformScalarNMacroFieldWidget : public DynamicUniformFieldWidgetBase
	{
	public:
		DynamicUniformScalarNMacroFieldWidget(DynamicUniformScalarNMacroField* field);

		virtual void Draw() override;

	private:
		DynamicUniformScalarNMacroField* m_Field;
	};

	class DynamicUniformColorFieldWidget : public DynamicUniformFieldWidgetBase
	{
	public:
		DynamicUniformColorFieldWidget(DynamicUniformColorField* field);

		virtual void Draw() override;

	private:
		DynamicUniformColorField* m_Field;
	};

	class DynamicUniformTexture2DFieldWidget : public DynamicUniformFieldWidgetBase
	{
	public:
		DynamicUniformTexture2DFieldWidget(DynamicUniformTexture2DField* field);

		virtual void Draw() override;

	private:
		DynamicUniformTexture2DField* m_Field;
		AssetBrowser m_Browser{ Texture2D::TypeID() };
	};
	
}
