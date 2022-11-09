#include "Inspectors/AssetInspector.h"

#include <IconsFontAwesome5.h>

#include "Core/Editor.h"
#include "Engine/ImGui/MyImGui.h"
#include "Panels/InspectorPanel.h"
#include "Worlds/AssetPreviewWorlds.h"

namespace ZeoEngine {

	AssetInspector::AssetInspector(AssetPreviewWorldBase* world, U32 compID)
		: ComponentInspector(compID)
		, m_AssetWorld(world)
	{
	}

	void AssetInspector::Draw(Entity entity)
	{
		auto* inspectorPanel = g_Editor->GetPanel<InspectorPanel>(INSPECTOR);
		if (inspectorPanel->IsLastInspectHistoryValid() && ImGui::TransparentButton(ICON_FA_ARROW_LEFT))
		{
			inspectorPanel->InspectLast();
		}
		if (inspectorPanel->IsLastInspectHistoryValid() && ImGui::IsItemHovered())
		{
			const std::string lastName = inspectorPanel->GetLastInspectHistoryDisplayName();
			ImGui::SetTooltipWithPadding(lastName.c_str());
		}

		ImGui::SameLine(0.0f, 0.0f);

		if (ImGui::TransparentButton(ICON_FA_SAVE))
		{
			m_AssetWorld->SaveCurrentAsset();
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltipWithPadding("Save asset");
		}

		ImGui::SameLine(0.0f, 0.0f);

		if (ImGui::TransparentButton(ICON_FA_FILE_EXPORT))
		{
			m_AssetWorld->SaveAssetAs();
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltipWithPadding("Save asset as");
		}

		ImGui::Separator();

		if (ImGui::BeginChild("AssetInspector"))
		{
			const auto tableID = ImGui::GetID("");
			SetTableID(tableID);
			// Push entity ID
			ImGui::PushID(static_cast<U32>(entity));
			{
				ComponentInspector::Draw(entity);
				ProcessDraw(entity);
			}
			ImGui::PopID();
		}
		ImGui::EndChild();
	}
	
}
