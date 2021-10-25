#include "Panels/SaveAssetPanel.h"

#include "Engine/Core/AssetRegistry.h"
#include "Engine/Core/AssetManager.h"
#include "Engine/Core/AssetFactory.h"
#include "Engine/Utils/PathUtils.h"
#include "Editors/EditorBase.h"

namespace ZeoEngine {

	void SaveAssetPanel::OnPanelOpen()
	{
		m_bHasKeyboardFocused = false;
		strcpy_s(m_NameBuffer, "New");
		auto typeName = AssetManager::Get().GetAssetFactoryByAssetType(GetAssetTypeId())->GetAssetTypeName();
		std::string formattedName = GetFormatedAssetTypeName(typeName);
		strcat_s(m_NameBuffer, formattedName.c_str());
	}

	void SaveAssetPanel::OnPathSelected(const std::string& path)
	{
		auto spec = AssetRegistry::Get().GetPathSpec(path);
		if (spec->IsAsset())
		{
			std::string name = PathUtils::GetNameFromPath(path);
			strcpy_s(m_NameBuffer, name.c_str());
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

		std::string newPath;
		ImGui::BeginDisabled(strlen(m_NameBuffer) == 0);
		{
			if (ImGui::Button("Save"))
			{
				strcat_s(m_NameBuffer, AssetRegistry::GetEngineAssetExtension());
				newPath = PathUtils::AppendPath(GetSelectedDirectory(), m_NameBuffer);
				if (AssetRegistry::Get().ContainsPathInDirectory(GetSelectedDirectory(), newPath))
				{
					m_ToReplacePath = std::move(newPath);
				}
				else
				{
					// Create an empty asset
					RequestPathCreation(newPath, GetAssetTypeId(), false);
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

	void SaveAssetPanel::HandleRightColumnAssetOpen(const std::string& path)
	{
		m_ToReplacePath = path;
	}

	void SaveAssetPanel::DrawReplaceDialog(const std::string& path)
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
