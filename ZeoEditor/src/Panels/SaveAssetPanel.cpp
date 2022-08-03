#include "Panels/SaveAssetPanel.h"

#include "Core/EditorManager.h"
#include "Panels/ContentBrowserPanel.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Editors/EditorBase.h"
#include "EditorUIRenderers/EditorUIRendererBase.h"

namespace ZeoEngine {

	void SaveAssetPanel::OnPanelOpen()
	{
		m_bHasKeyboardFocused = false;
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(GetContextEditor()->GetAsset()->GetHandle());
		strcpy_s(m_NameBuffer, metadata->PathName.c_str());
		SetSelectedDirectory(metadata->Path.parent_path());
	}

	void SaveAssetPanel::OnPathSelected(const std::filesystem::path& path)
	{
		if (AssetRegistry::Get().GetAssetMetadata(path))
		{
			strcpy_s(m_NameBuffer, path.stem().string().c_str());
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
				const auto metadata = AssetRegistry::Get().GetAssetMetadata(GetContextEditor()->GetAsset()->GetHandle());
				auto name = std::filesystem::path(m_NameBuffer);
				const auto originalExtension = std::filesystem::path(metadata->PathName).extension();
				if (name.extension() != originalExtension)
				{
					// Force add resource extension if not exist
					// For non-resource asset, this should never happen
					name += originalExtension;
				}
				auto newPath = GetSelectedDirectory() / name;
				newPath += AssetRegistry::GetEngineAssetExtension();
				if (AssetRegistry::Get().ContainsPathInDirectory(GetSelectedDirectory(), newPath))
				{
					m_ToReplacePath = std::move(newPath);
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
			DrawReplaceDialog(m_ToReplacePath);
		}
	}

	void SaveAssetPanel::HandleRightColumnAssetOpen(const std::filesystem::path& path)
	{
		m_ToReplacePath = path;
	}

	void SaveAssetPanel::DrawReplaceDialog(const std::filesystem::path& path)
	{
		static ImVec2 buttonSize{ 120.0f, 0.0f };
		ImGui::OpenPopup("Replace?");
		if (ImGui::BeginPopupModal("Replace?", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Text("The asset already exists.\nDo you want to replace it?\n\n");

			if (ImGui::Button("OK", buttonSize))
			{
				SaveAndClose(path);
				m_ToReplacePath.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();

			ImGui::SameLine();

			if (ImGui::Button("Cancel", buttonSize))
			{
				m_ToReplacePath.clear();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void SaveAssetPanel::SaveAndClose(const std::filesystem::path& path)
	{
		GetContextEditor()->SaveScene(path);
		Close();
		const auto levelEditor = EditorManager::Get().GetEditor(LEVEL_EDITOR);
		const auto contentBrowser = levelEditor->GetEditorUIRenderer()->GetPanel<ContentBrowserPanel>(CONTENT_BROWSER);
		// Jump and focus new saved asset
		contentBrowser->SetSelectedPath(path);
	}
}
