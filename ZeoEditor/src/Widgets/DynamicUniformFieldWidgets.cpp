#include "Widgets/DynamicUniformFieldWidgets.h"

namespace ZeoEngine {

	DynamicUniformBoolFieldWidget::DynamicUniformBoolFieldWidget(DynamicUniformBoolFieldBase* field)
		: m_Field(field)
	{
	}

	void DynamicUniformBoolFieldWidget::Draw()
	{
		bool bValue = m_Field->Value;
		if (ImGui::Checkbox("", &bValue))
		{
			m_Field->Value = bValue;
			m_Field->Apply();
		}
	}

	DynamicUniformScalarNMacroFieldWidget::DynamicUniformScalarNMacroFieldWidget(DynamicUniformScalarNMacroField* field)
		: m_Field(field)
	{
	}

	void DynamicUniformScalarNMacroFieldWidget::Draw()
	{
		const I32 min = 0;
		const I32 max = m_Field->ValueRange - 1;
		void* buffer = m_Field->bIsEditActive ? &m_Field->Buffer : &m_Field->Value;
		// We do not apply during dragging as reloading and reconstructing widgets are not necessary during this operation
		ImGui::DragScalarNEx("", ImGuiDataType_S32, buffer, 1, 0.5f, &min, &max, "%d", ImGuiSliderFlags_AlwaysClamp);
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

	DynamicUniformColorFieldWidget::DynamicUniformColorFieldWidget(DynamicUniformColorField* field)
		: m_Field(field)
	{
	}

	void DynamicUniformColorFieldWidget::Draw()
	{
		float* buffer = m_Field->bIsEditActive ? glm::value_ptr(m_Field->Buffer) : glm::value_ptr(m_Field->Value);
		bool bChanged = ImGui::ColorEdit4("", buffer);
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

	DynamicUniformTexture2DFieldWidget::DynamicUniformTexture2DFieldWidget(DynamicUniformTexture2DField* field)
		: m_Field(field)
	{
	}

	void DynamicUniformTexture2DFieldWidget::Draw()
	{
		if (m_Browser.Draw(m_Field->Value, 0.0f, []() {}))
		{
			m_Field->Apply();
		}
	}

}
