#include "Panels/AssetViewPanel.h"

#include "Worlds/AssetPreviewWorlds.h"
#include "Panels/AssetViews.h"

namespace ZeoEngine {

	AssetViewPanel::AssetViewPanel(std::string panelName)
		: ViewPanelBase(std::move(panelName))
	{
		SetFlags(ImGuiWindowFlags_NoFocusOnAppearing);
		SetDisableClose(true);
	}

	void AssetViewPanel::ProcessRender()
	{
		const auto startPos = ImGui::GetCursorPos();

		ViewPanelBase::ProcessRender();

		GetEditorWorld()->GetAssetView()->Draw();

		ImGui::SetCursorPos(startPos);
		GetEditorWorld()->GetAssetView()->DrawToolBar();
	}

}
