#include "Panels/SaveAssetPanel.h"

#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Asset/AssetFactory.h"
#include "Engine/Utils/PathUtils.h"
#include "Editors/EditorBase.h"

namespace ZeoEngine {

	void SaveAssetPanel::OnPanelOpen()
	{
		m_bHasKeyboardFocused = false;
		strcpy_s(m_NameBuffer, "New");
		const auto typeName = AssetManager::Get().GetAssetFactoryByAssetType(GetAssetTypeID())->GetAssetTypeName();
		const std::string formattedName = GetFormattedAssetTypeName(typeName);
		strcat_s(m_NameBuffer, formattedName.c_str());
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
				strcat_s(m_NameBuffer, AssetRegistry::GetEngineAssetExtension());
				auto newPath = GetSelectedDirectory() / m_NameBuffer;
				if (AssetRegistry::Get().ContainsPathInDirectory(GetSelectedDirectory(), newPath))
				{
					m_ToReplacePath = std::move(newPath);
				}
				else
				{
					// Create an empty asset
					RequestPathCreation(newPath, GetAssetTypeID(), false);
					// Serialize data
					GetContextEditor()->SaveScene(newPath);
					Close();
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
				GetContextEditor()->SaveScene(path);
				Close();

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

}
