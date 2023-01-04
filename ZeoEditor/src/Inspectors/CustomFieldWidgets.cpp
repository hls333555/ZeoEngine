#include "Inspectors/FieldWidget.h"
#include "Engine/ImGui/ScriptClassBrowser.h"

namespace ZeoEngine {

	template<typename FieldInstance>
	class ScriptClassFieldWidget : public FieldWidgetBase<FieldInstance>
	{
	public:
		using FieldWidgetBase<FieldInstance>::FieldWidgetBase;

	private:
		virtual void ProcessDraw() override
		{
			const auto fieldInstance = this->GetFieldInstance();
			auto value = fieldInstance->GetValue<std::string>();
			float rightPadding = 0.0f;
			if constexpr (Utils::IsFieldSequenceContainer<FieldInstance>())
			{
				rightPadding = Utils::GetContainerDropdownWidth();
			}
			if (m_Browser.Draw(value, rightPadding))
			{
				this->ApplyValueToInstance(&value);
			}
		}

		virtual void ApplyValueToInstance(const void* value) override
		{
			const auto fieldInstance = this->GetFieldInstance();
			const auto oldValueStr = fieldInstance->GetValue<std::string>(); // Copy string
			const auto& valueStr = *static_cast<const std::string*>(value);
			if (valueStr == oldValueStr) return;

			fieldInstance->SetValue(valueStr);

			const U32 fieldID = fieldInstance->GetFieldID();
			fieldInstance->OnFieldValueChanged(fieldID);
		}

	private:
		ScriptClassBrowser m_Browser;
	};

	Scope<class IFieldWidget> ConstructScriptClassFieldWidget(U32 widgetID, Ref<ComponentFieldInstance> fieldInstance)
	{
		return CreateScope<ScriptClassFieldWidget<ComponentFieldInstance>>(widgetID, std::move(fieldInstance));
	}
	
}
