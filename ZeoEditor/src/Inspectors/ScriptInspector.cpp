#include "Inspectors/ScriptInspector.h"

#include <imgui.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Scripting/ScriptEngine.h"
#include "Inspectors/FieldWidget.h"

namespace ZeoEngine {

	ScriptInspector::ScriptInspector(U32 compID)
		: ComponentInspector(compID)
	{
	}

	ScriptInspector::~ScriptInspector() = default;

	void ScriptInspector::DrawExtraFieldWidgets(Entity entity)
	{
		const UUID entityID = entity.GetUUID();
		const auto& className = entity.GetComponent<ScriptComponent>().ClassName;
		if (ScriptEngine::EntityClassExists(className))
		{
			auto& entityFields = ScriptEngine::GetScriptFieldMap(entityID);
			for (const auto& [name, field] : entityFields)
			{
				// Push field name as ID
				ImGui::PushID(name.c_str());
				{
					DrawScriptFieldWidget(field);
				}
				ImGui::PopID();
			}
		}
	}

	void ScriptInspector::DrawScriptFieldWidget(const Ref<ScriptFieldInstance>& fieldInstance)
	{
		const U32 aggregatedID = ImGui::GetCurrentWindow()->GetID(fieldInstance->GetFieldName());
		if (m_ScriptFieldWidgets.find(aggregatedID) != m_ScriptFieldWidgets.cend())
		{
			if (m_ScriptFieldWidgets[aggregatedID])
			{
				m_ScriptFieldWidgets[aggregatedID]->Draw();
			}
		}
		else
		{
			m_ScriptFieldWidgets[aggregatedID] = Utils::ConstructFieldWidget<ScriptFieldInstance>(fieldInstance->GetFieldType(), aggregatedID, fieldInstance);
		}
	}

}
