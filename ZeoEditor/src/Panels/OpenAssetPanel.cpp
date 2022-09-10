#include "Panels/OpenAssetPanel.h"

#include "Engine/Asset/AssetRegistry.h"
#include "Core/Editor.h"

namespace ZeoEngine {

	OpenAssetPanel::OpenAssetPanel(std::string panelName, AssetTypeID assetTypeID)
		: AssetBrowserPanelBase(std::move(panelName))
		, m_AssetTypeID(assetTypeID)
	{
	}

	void OpenAssetPanel::OnAttach()
	{
		AssetBrowserPanelBase::OnAttach();

		m_PanelSpec.WindowFlags |= ImGuiWindowFlags_NoDocking;
		m_PanelSpec.InitialSize = { { 700.0f, 400.0f } };
	}

	void OpenAssetPanel::DrawBottom()
	{
		const auto& selectedPath = GetSelectedPath();
		ImGui::BeginDisabled(selectedPath.empty());
		{
			static const char* buttonText = "Open";
			const float paddingRight = ImGui::GetFontSize();
			const float availX = ImGui::GetContentRegionAvail().x;
			const float textWidth = ImGui::CalcTextSize(buttonText).x;
			// Align to the right
			ImGui::Indent(availX - textWidth - ImGui::GetFramePadding().x * 2 - paddingRight);
			if (ImGui::Button(buttonText))
			{
				RequestPathOpen(selectedPath);
			}
		}
		ImGui::EndDisabled();
	}

	bool OpenAssetPanel::ShouldDrawPath(const Ref<PathMetadata>& metadata)
	{
		const auto assetTypeID = metadata->GetAssetTypeID();
		// Directory or specific asset
		return assetTypeID == AssetTypeID{} || assetTypeID == m_AssetTypeID;
	}

	void OpenAssetPanel::HandleRightColumnAssetOpen(const std::string& path)
	{
		g_Editor->LoadLevel(path);
		Toggle(false);
	}

}
