#include "Panels/ContentBrowserPanel.h"

#include "Engine/Debug/BenchmarkTimer.h"
#include "Engine/Core/Assert.h"
#include "Engine/Utils/PathUtils.h"
#include "Engine/Core/AssetManager.h"
#include "Engine/Core/AssetFactory.h"
#include "Engine/Core/AssetActions.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/Core/ThumbnailManager.h"
#include "Engine/Utils/PlatformUtils.h"

namespace ZeoEngine {

	void ContentBrowserPanel::OnAttach()
	{
		AssetBrowserPanelBase::OnAttach();

		InitAssetTypeFilters();
	}

	void ContentBrowserPanel::InitAssetTypeFilters()
	{
		AssetManager::Get().ForEachAssetFactory([this](AssetTypeId typeId, const Ref<IAssetFactory>& factory)
		{
			m_AssetTypeFilters.emplace_back(AssetTypeFilterSpec{ typeId, factory->GetAssetTypeName(), false });
		});
	}

	void ContentBrowserPanel::DrawTopBar()
	{
		// Import an asset via file dialog
		if (ImGui::Button(ICON_FA_FILE_IMPORT " Import"))
		{
			auto filePath = FileDialogs::OpenFile();
			if (filePath)
			{
				std::string extension = PathUtils::GetExtensionFromPath(*filePath);
				std::string assetName = PathUtils::GetFileNameFromPath(*filePath);
				std::string destPath = PathUtils::AppendPath(GetSelectedDirectory(), assetName);
				auto& am = AssetManager::Get();
				am.ImportAsset(*am.GetTypdIdFromFileExtension(extension), PathUtils::GetRelativePath(*filePath), destPath);
				SetForceUpdateFilterCache(true);
			}
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltipWithPadding("Import to %s", GetSelectedDirectory().c_str());
		}

		ImGui::SameLine();

		m_bIsTypeFilterChanged = false;
		// Filters menu
		if (ImGui::BeginPopupWithPadding("Filters"))
		{
			m_bIsAnyTypeFilterActive = false;
			for (auto& filterSpec : m_AssetTypeFilters)
			{
				bool bIsCheckChanged = ImGui::Checkbox(filterSpec.TypeName, &filterSpec.bIsFilterActive);
				if (filterSpec.bIsFilterActive)
				{
					m_bIsAnyTypeFilterActive = true;
				}
				if (bIsCheckChanged)
				{
					m_bIsTypeFilterChanged = true;
				}
			}
			
			ImGui::EndPopup();
		}
		if (ImGui::Button(ICON_FA_FILTER " Filters"))
		{
			ImGui::OpenPopup("Filters");
		}

		ImGui::SameLine();

		AssetBrowserPanelBase::DrawTopBar();
	}

	bool ContentBrowserPanel::PassFilter(const Ref<PathSpec>& spec) const
	{
		if (m_bIsAnyTypeFilterActive)
		{
			auto typeId = spec->GetAssetTypeId();
			auto it = std::find_if(m_AssetTypeFilters.begin(), m_AssetTypeFilters.end(), [typeId](const auto& filterSpec)
			{
				return filterSpec.TypeId == typeId;
			});
			return it->bIsFilterActive;
		}

		return true;
	}

	void ContentBrowserPanel::ClearAllFilters()
	{
		AssetBrowserPanelBase::ClearAllFilters();

		for (auto& typeFilter : m_AssetTypeFilters)
		{
			typeFilter.bIsFilterActive = false;
		}
		m_bIsAnyTypeFilterActive = false;
		m_bIsTypeFilterChanged = true;
	}

	void ContentBrowserPanel::DrawWindowContextMenuImpl(float thumbnailWidth)
	{
		ImGui::Separator();

		AssetManager::Get().ForEachAssetFactory([this, thumbnailWidth](AssetTypeId typeId, const Ref<IAssetFactory>& factory)
		{
			if (!factory->ShouldShowInContextMenu()) return;

			// Push asset type id
			ImGui::PushID(typeId);
			{
				bool bIsAssetCreationSelected = ImGui::Selectable("##AssetCreationSelectable", false, 0, ImVec2(0.0f, thumbnailWidth));

				ImGui::SameLine();

				// Draw asset type icon
				ImGui::Image(ThumbnailManager::Get().GetAssetTypeIcon(typeId)->GetTextureID(),
					{ thumbnailWidth, thumbnailWidth },
					{ 0.0f, 1.0f }, { 1.0f, 0.0f });

				ImGui::SameLine();

				ImGui::AlignTextToFramePadding();
				const char* assetTypeName = factory->GetAssetTypeName();
				ImGui::Text(assetTypeName);

				if (bIsAssetCreationSelected)
				{
					char baseName[GetMaxPathSize()] = "New";
					std::string formattedName = GetFormatedAssetTypeName(assetTypeName);
					strcat_s(baseName, formattedName.c_str());
					std::string newPath = GetAvailableNewPathName(baseName, true);
					RequestPathCreation(newPath, typeId, true);
				}
			}
			ImGui::PopID();
		});
	}

	void ContentBrowserPanel::DrawPathContextMenuItem_Save(const std::string& path, bool bIsAsset)
	{
		if (bIsAsset)
		{
			if (ImGui::MenuItem("Save", "CTRL+S"))
			{
				AssetManager::Get().SaveAsset(path);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Save this asset");
			}
		}
		else
		{
			if (ImGui::MenuItem("Save All", "CTRL+S"))
			{
				AssetRegistry::Get().ForEachPathInDirectoryRecursively(path, [](const std::string& inPath)
				{
					auto spec = AssetRegistry::Get().GetPathSpec(inPath);
					if (spec->IsAsset())
					{
						AssetManager::Get().SaveAsset(inPath);
					}
				});
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Save all assets in this folder");
			}
		}
	}

	void ContentBrowserPanel::DrawPathContextMenuItem_Asset(const std::string& path, const Ref<PathSpec>& spec)
	{
		AssetManager& AM = AssetManager::Get();

		if (ImGui::MenuItem("Edit"))
		{
			AM.OpenAsset(path);
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltipWithPadding("Open asset editor");
		}

		if (ImGui::MenuItem("Reload"))
		{
			AM.ReloadAsset(path);
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltipWithPadding("Discard changes and reload from disk");
		}
		if (spec->IsImportableAsset())
		{
			if (ImGui::MenuItem("Reimport"))
			{
				AM.ReimportAsset(path);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Reimport from its original place");
			}
		}

		ImGui::Separator();
	}

	void ContentBrowserPanel::ProcessAssetDragging(const Ref<PathSpec>& spec, float thumbnailRounding)
	{
		ImGui::PushStyleColor(ImGuiCol_PopupBg, { 0.0f, 0.0f, 0.0f, 0.0f });
		if (spec->IsAsset() && ImGui::BeginDragDropSource())
		{
			char typeStr[32];
			_itoa_s(spec->GetAssetTypeId(), typeStr, 10);
			ImGui::SetDragDropPayload(typeStr, &spec, sizeof(spec));

			// Draw tooltip thumbnail
			ImGui::AssetThumbnail(spec->ThumbnailTexture->GetTextureID(),
				GetTileThumbnailWidth(), thumbnailRounding,
				true, Texture2D::s_DefaultBackgroundTexture->GetTextureID());

			ImGui::EndDragDropSource();
		}
		ImGui::PopStyleColor();
	}

	void ContentBrowserPanel::HandleRightColumnAssetOpen(const std::string& path)
	{
		AssetManager::Get().OpenAsset(path);
	}

}
