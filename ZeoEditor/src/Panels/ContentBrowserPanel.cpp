#include "Panels/ContentBrowserPanel.h"

#include "Engine/Profile/BenchmarkTimer.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Asset/AssetFactory.h"
#include "Engine/Asset/AssetActions.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Core/ThumbnailManager.h"
#include "Engine/Utils/PlatformUtils.h"
#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	void ContentBrowserPanel::OnAttach()
	{
		AssetBrowserPanelBase::OnAttach();

		InitAssetTypeFilters();
	}

	void ContentBrowserPanel::ProcessEvent(Event& e)
	{
		AssetBrowserPanelBase::ProcessEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowFileDroppedEvent>(ZE_BIND_EVENT_FUNC(ContentBrowserPanel::OnFileDropped));
	}

	bool ContentBrowserPanel::OnFileDropped(WindowFileDroppedEvent& e)
	{
		if (!IsPanelHovered()) return false;

		for (const auto& filePath : e.GetPaths())
		{
			ImportAsset(filePath);
		}

		return true;
	}

	void ContentBrowserPanel::InitAssetTypeFilters()
	{
		AssetManager::Get().ForEachAssetFactory([this](AssetTypeID typeID, AssetFactoryBase* factory)
		{
			m_AssetTypeInfos.emplace_back(AssetTypeFilterInfo{ typeID, factory->GetAssetTypeName(), false });
		});
	}

	void ContentBrowserPanel::DrawTopBar()
	{
		// Import assets via file dialog
		if (ImGui::Button(ICON_FA_FILE_IMPORT " Import"))
		{
			const auto filePaths = FileDialogs::Open(true);
			for (const auto& filePath : filePaths)
			{
				ImportAsset(filePath);
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
			for (auto& filterInfo : m_AssetTypeInfos)
			{
				const bool bIsCheckChanged = ImGui::Checkbox(filterInfo.TypeName, &filterInfo.bIsFilterActive);
				if (filterInfo.bIsFilterActive)
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

	bool ContentBrowserPanel::PassFilter(const Ref<PathMetadata>& metadata) const
	{
		if (m_bIsAnyTypeFilterActive)
		{
			const auto typeID = metadata->GetAssetTypeID();
			const auto it = std::find_if(m_AssetTypeInfos.begin(), m_AssetTypeInfos.end(), [typeID](const auto& filterInfo)
			{
				return filterInfo.TypeID == typeID;
			});
			return it->bIsFilterActive;
		}

		return true;
	}

	void ContentBrowserPanel::ClearAllFilters()
	{
		AssetBrowserPanelBase::ClearAllFilters();

		for (auto& typeFilter : m_AssetTypeInfos)
		{
			typeFilter.bIsFilterActive = false;
		}
		m_bIsAnyTypeFilterActive = false;
		m_bIsTypeFilterChanged = true;
	}

	void ContentBrowserPanel::DrawWindowContextMenuImpl(float thumbnailWidth)
	{
		ImGui::Separator();

		AssetManager::Get().ForEachAssetFactory([this, thumbnailWidth](AssetTypeID typeID, AssetFactoryBase* factory)
		{
			if (!factory->ShouldShowInContextMenu()) return;

			// Push asset type id
			ImGui::PushID(typeID);
			{
				const bool bIsAssetCreationSelected = ImGui::Selectable("##AssetCreationSelectable", false, 0, ImVec2(0.0f, thumbnailWidth));

				ImGui::SameLine();

				// Draw asset type icon
				ImGui::Image(ThumbnailManager::Get().GetAssetTypeIcon(typeID)->GetTextureID(),
					{ thumbnailWidth, thumbnailWidth },
					{ 0.0f, 1.0f }, { 1.0f, 0.0f });

				ImGui::SameLine();

				ImGui::AlignTextToFramePadding();
				const char* assetTypeName = factory->GetAssetTypeName();
				ImGui::Text(assetTypeName);

				if (bIsAssetCreationSelected)
				{
					char baseName[MAX_PATH_SIZE] = "New";
					strcat_s(baseName, assetTypeName);
					strcat_s(baseName, factory->GetResourceExtension());
					const std::string formattedName = GetFormattedAssetTypeName(baseName);
					const auto newPath = GetAvailableNewPathName(formattedName.c_str(), true);
					RequestPathCreation(newPath, typeID, true);
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
				if (AssetLibrary::HasAsset(path))
				{
					const auto asset = AssetLibrary::LoadAsset<IAsset>(path);
					AssetManager::Get().SaveAsset(path, asset);
				}
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
					if (AssetRegistry::Get().GetAssetMetadata(inPath))
					{
						const auto asset = AssetLibrary::LoadAsset<IAsset>(inPath);
						AssetManager::Get().SaveAsset(inPath, asset);
					}
				});
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Save all assets in this folder");
			}
		}
	}

	void ContentBrowserPanel::DrawPathContextMenuItem_Asset(const std::string& path, const Ref<AssetMetadata>& metadata)
	{
		const auto& am = AssetManager::Get();

		if (ImGui::MenuItem("Edit"))
		{
			am.OpenAsset(path);
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltipWithPadding("Open asset editor");
		}

		if (ImGui::MenuItem("Reload"))
		{
			AssetLibrary::ReloadAsset(path);
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltipWithPadding("Discard changes and reload from disk");
		}
		if (metadata->IsImportableAsset())
		{
			if (ImGui::MenuItem("Reimport"))
			{
				am.ReimportAsset(path);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Reimport from its original place");
			}
		}

		ImGui::Separator();
	}

	void ContentBrowserPanel::ImportAsset(const std::string& path)
	{
		const auto destPath = fmt::format("{}/{}", GetSelectedDirectory(), PathUtils::GetPathFileName(path));
		const auto& am = AssetManager::Get();
		if (const auto typeID = am.GetAssetTypeFromFileExtension(PathUtils::GetPathExtension(path)))
		{
			am.ImportAsset(typeID, path, destPath);
			SetForceUpdateFilterCache(true);
		}
	}

	void ContentBrowserPanel::ProcessAssetDragging(const Ref<PathMetadata>& metadata, float thumbnailRounding)
	{
		ImGui::PushStyleColor(ImGuiCol_PopupBg, { 0.0f, 0.0f, 0.0f, 0.0f });
		if (metadata->IsAsset() && ImGui::BeginDragDropSource())
		{
			char typeStr[DRAG_DROP_PAYLOAD_TYPE_SIZE];
			_itoa_s(metadata->GetAssetTypeID(), typeStr, 10);
			ImGui::SetDragDropPayload(typeStr, &metadata, sizeof(metadata));

			// Draw tooltip thumbnail
			ImGui::AssetThumbnail(metadata->ThumbnailTexture->GetTextureID(),
				GetTileThumbnailWidth(), thumbnailRounding,
				true, Texture2D::GetAssetBackgroundTexture()->GetTextureID());

			ImGui::EndDragDropSource();
		}
		ImGui::PopStyleColor();
	}

	void ContentBrowserPanel::HandleRightColumnAssetOpen(const std::string& path)
	{
		AssetManager::Get().OpenAsset(path);
	}

}
