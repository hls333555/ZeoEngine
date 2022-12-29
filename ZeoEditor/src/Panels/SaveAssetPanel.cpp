#include "Panels/SaveAssetPanel.h"

#include "Core/Editor.h"
#include "Panels/ContentBrowserPanel.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Core/CommonPaths.h"
#include "Engine/ImGui/MyImGui.h"
#include "Worlds/AssetPreviewWorlds.h"

namespace ZeoEngine {

	SaveAssetPanel::SaveAssetPanel(std::string panelName, AssetTypeID assetTypeID, EditorPreviewWorldBase* world)
		: OpenAssetPanel(std::move(panelName), assetTypeID)
		, m_World(world)
	{
	}

	void SaveAssetPanel::OnPanelOpen()
	{
		m_bHasKeyboardFocused = false;
		const AssetHandle handle = m_World->GetAsset()->GetHandle();
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(handle);
		strcpy_s(m_NameBuffer, metadata->PathName.c_str());
		SetSelectedDirectory(metadata->IsTemplateAsset() ? CommonPaths::GetProjectAssetDirectoryStandard() : FileSystemUtils::GetParentPath(metadata->Path));
	}

	void SaveAssetPanel::OnPathSelected(const std::string& path)
	{
		if (AssetRegistry::Get().GetAssetMetadata(path))
		{
			strcpy_s(m_NameBuffer, FileSystemUtils::GetPathName(path).c_str());
		}
	}

	void SaveAssetPanel::DrawBottom()
	{
		ImGui::Text(" Name: ");
		
		ImGui::SameLine();

		if (!m_bHasKeyboardFocused)
		{
			// Focus input once
			ImGui::SetKeyboardFocusHere();
			m_bHasKeyboardFocused = true;
		}

		const float paddingRight = ImGui::GetFontSize();
		const float availX = ImGui::GetContentRegionAvail().x;
		const float textWidth = ImGui::CalcTextSize("Open").x;
		ImGui::SetNextItemWidth(availX - textWidth - ImGui::GetFramePadding().x * 2 - paddingRight);
		ImGui::InputText("##NamePath", m_NameBuffer, sizeof(m_NameBuffer), ImGuiInputTextFlags_AutoSelectAll);

		ImGui::SameLine();

		ImGui::BeginDisabled(strlen(m_NameBuffer) == 0);
		{
			if (ImGui::Button("Save"))
			{
				const AssetHandle handle = m_World->GetAsset()->GetHandle();
				const auto metadata = AssetRegistry::Get().GetAssetMetadata(handle);
				std::string name = m_NameBuffer;
				const std::string extension = FileSystemUtils::GetPathExtension(name);
				const std::string originalExtension = FileSystemUtils::GetPathExtension(metadata->PathName);
				if (extension != originalExtension)
				{
					// Force add resource extension if not exist
					// For non-resource asset, this should never happen
					name += originalExtension;
				}
				std::string newPath = fmt::format("{}/{}{}", GetSelectedDirectory(), std::move(name), AssetRegistry::GetAssetExtension());
				if (AssetRegistry::Get().ContainsPathInDirectory(GetSelectedDirectory(), newPath))
				{
					if (AssetRegistry::Get().GetAssetMetadata(newPath)->TypeID == metadata->TypeID)
					{
						m_ToReplacePath = std::move(newPath);
					}
					else
					{
						m_bReplaceError = true;
					}
				}
				else
				{
					if (metadata->IsResourceAsset())
					{
						// Copy resource and asset
						RequestPathCreationForResourceAsset(metadata->Path, newPath);
					}
					else
					{
						// Create an empty asset
						RequestPathCreation(newPath, GetAssetTypeID(), false);
					}
					// Serialize data
					SaveAndClose(newPath);
				}
			}
		}
		ImGui::EndDisabled();

		if (!m_ToReplacePath.empty())
		{
			DrawReplaceDialog();
		}

		if (m_bReplaceError)
		{
			DrawReplaceErrorDialog();
		}
	}

	void SaveAssetPanel::HandleRightColumnAssetOpen(const std::string& path)
	{
		m_ToReplacePath = path;
	}

	static ImVec2 s_ButtonSize{ 120.0f, 0.0f };

	void SaveAssetPanel::DrawReplaceDialog()
	{
		ImGui::OpenPopup("Replace?");
		if (ImGui::BeginPopupModal("Replace?", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Text("The asset already exists.\nDo you want to replace it?\n\n");

			if (ImGui::Button("OK", s_ButtonSize))
			{
				SaveAndClose(m_ToReplacePath);
				m_ToReplacePath.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();

			ImGui::SameLine();

			if (ImGui::Button("Cancel", s_ButtonSize))
			{
				m_ToReplacePath.clear();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void SaveAssetPanel::DrawReplaceErrorDialog()
	{
		ImGui::OpenPopup("Replace Error");
		if (ImGui::BeginPopupModal("Replace Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Text("The asset with a different type already exists! Please choose another name.\n\n");

			ImGui::Indent((ImGui::GetContentRegionAvail().x  - s_ButtonSize.x) / 2);

			if (ImGui::Button("OK", s_ButtonSize))
			{
				m_bReplaceError = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void SaveAssetPanel::SaveAndClose(const std::string& path)
	{
		m_World->SaveAsset(path);
		const AssetHandle handle = m_World->GetAsset()->GetHandle();
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(handle);
		if (path != metadata->Path)
		{
			m_World->OnAssetSaveAs(path);
		}
		Toggle(false);
		auto* contentBrowser = g_Editor->GetPanel<ContentBrowserPanel>(CONTENT_BROWSER);
		// Jump and focus new saved asset
		contentBrowser->SetSelectedPath(path);
	}

}
