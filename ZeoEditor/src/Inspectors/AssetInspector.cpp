#include "Inspectors/AssetInspector.h"

#include <IconsFontAwesome5.h>

#include "Core/Editor.h"
#include "Engine/ImGui/MyImGui.h"
#include "Panels/InspectorPanel.h"
#include "Worlds/AssetPreviewWorlds.h"

namespace ZeoEngine {

	AssetInspector::AssetInspector(const Ref<AssetPreviewWorldBase>& world, U32 compId)
		: ComponentInspector(compId)
		, m_World(world.get())
	{
	}

	void AssetInspector::Draw(Entity entity)
	{
		const auto inspectorPanel = g_Editor->GetPanel<InspectorPanel>(INSPECTOR);
		if (!inspectorPanel->IsInspectHistoryEmpty() && ImGui::TransparentButton(ICON_FA_ARROW_LEFT))
		{
			inspectorPanel->InspectLast();
		}
		if (!inspectorPanel->IsInspectHistoryEmpty() && ImGui::IsItemHovered())
		{
			const std::string lastName = inspectorPanel->GetLastInspectHistoryDisplayName();
			ImGui::SetTooltipWithPadding(lastName.c_str());
		}

		ImGui::SameLine(0.0f, 0.0f);

		if (ImGui::TransparentButton(ICON_FA_SAVE))
		{
			m_World->SaveCurrentAsset();
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltipWithPadding("Save asset");
		}

		ImGui::SameLine(0.0f, 0.0f);

		if (ImGui::TransparentButton(ICON_FA_FILE_EXPORT))
		{
			m_World->SaveAssetAs();
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltipWithPadding("Save asset as");
		}

		ImGui::Separator();

		if (ImGui::BeginChild("AssetInspector"))
		{
			// Push entity id
			ImGui::PushID(static_cast<U32>(entity));

			ComponentInspector::Draw(entity);
			ProcessDraw(entity);

			ImGui::PopID();
		}
		ImGui::EndChild();
	}
	
}
