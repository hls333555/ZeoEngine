#include "Inspectors/ScriptInspector.h"

#include <imgui.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Scripting/ScriptEngine.h"
#include "Widgets/FieldWidget.h"

namespace ZeoEngine {

	ScriptInspector::ScriptInspector(U32 compID)
		: ComponentInspector(compID)
	{
		if (auto* reloadDel= ScriptEngine::GetScriptReloadedDelegate())
		{
			reloadDel->connect<&ScriptInspector::ClearFieldsAndWidgets>(this);
		}
	}

	ScriptInspector::~ScriptInspector()
	{
		if (auto* reloadDel = ScriptEngine::GetScriptReloadedDelegate())
		{
			reloadDel->disconnect(this);
		}
	}

	void ScriptInspector::DrawExtraFieldWidgets(Entity entity, U32 backupItemID)
	{
		const auto& className = entity.GetComponent<ScriptComponent>().ClassName;
		if (ScriptEngine::EntityClassExists(className))
		{
			// When entity is changed, this ScriptInspector will be automatically recreated so we do not need to clear cache manually
			if (className != m_CurrentClassName)
			{
				ClearFieldsAndWidgets();
				m_CurrentClassName = className;
			}

			if (m_CategorizedFields.empty())
			{
				CategorizeFields(entity);
			}

			for (const auto& [category, fields] : m_CategorizedFields)
			{
				bool bShouldDisplayCategoryTree = false;

				std::vector<Ref<ScriptFieldInstance>> visibleFields;
				// Do not draw TreeNode if none of these fields will show or category is not set
				for (const auto& field : fields)
				{
					if (!field->IsHidden())
					{
						bShouldDisplayCategoryTree = !category.empty();
						visibleFields.emplace_back(field);
					}
				}

				bool bIsCategoryTreeExpanded = true;
				if (bShouldDisplayCategoryTree)
				{
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen/* Prevent indent of next row, which will affect column. */;
					// Field category tree
					bIsCategoryTreeExpanded = ImGui::TreeNodeEx(category.c_str(), flags);
				}
				if (bIsCategoryTreeExpanded)
				{
					if (!visibleFields.empty() && ImGui::BeginTable("", 2, ImGuiTableFlags_Resizable))
					{
						ImGui::TableNextColumn();

						ImGui::PushOverrideID(backupItemID);
						{
							for (const auto& field : visibleFields)
							{
								// Push field ID
								ImGui::PushID(field->GetFieldID());
								{
									DrawScriptFieldWidget(field);
								}
								ImGui::PopID();
							}
						}
						ImGui::PopID();

						ImGui::EndTable();
					}
				}
			}
		}
	}

	void ScriptInspector::DrawScriptFieldWidget(const Ref<ScriptFieldInstance>& fieldInstance)
	{
		const U32 fieldID = fieldInstance->GetFieldID();
		if (m_ScriptFieldWidgets.find(fieldID) != m_ScriptFieldWidgets.cend())
		{
			if (m_ScriptFieldWidgets[fieldID])
			{
				m_ScriptFieldWidgets[fieldID]->Draw();
			}
		}
		else
		{
			const FieldType type = fieldInstance->GetFieldType();
			const auto widgetID = UUID();
			if (type == FieldType::SeqCon)
			{
				m_ScriptFieldWidgets[fieldID] = Utils::ConstructFieldWidget(type, widgetID, std::static_pointer_cast<ScriptSequenceContainerFieldInstance>(fieldInstance));
			}
			else
			{
				m_ScriptFieldWidgets[fieldID] = Utils::ConstructFieldWidget(type, widgetID, fieldInstance);
			}
		}
	}

	void ScriptInspector::ClearFieldsAndWidgets()
	{
		m_ScriptFieldWidgets.clear();
		m_CategorizedFields.clear();
	}

	void ScriptInspector::CategorizeFields(Entity entity)
	{
		auto& entityFields = ScriptEngine::GetScriptFieldMap(entity.GetUUID());
		for (const auto& [name, field] : entityFields)
		{
			std::string category = field->GetCategory();
			m_CategorizedFields[std::move(category)].emplace_back(field);
		}
	}

}
