#include "Panels/AssetBrowserPanelBase.h"

#include <IconsFontAwesome5.h>
#include <iostream>

#include "Engine/Core/Assert.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Asset/AssetFactory.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Core/ThumbnailManager.h"
#include "Engine/ImGui/MyImGui.h"
#include "Engine/Profile/BenchmarkTimer.h"
#include "Engine/Utils/PlatformUtils.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	void AssetBrowserPanelBase::OnAttach()
	{
		m_SelectedDirectory = AssetRegistry::GetProjectPathPrefix();
	}

	void AssetBrowserPanelBase::ProcessRender()
	{
		DrawTopBar();

		ImGui::Separator();

		float contentWidth = ImGui::GetContentRegionAvail().x;

		bool bWillDrawBottom = WillDrawBottom();
		ImVec2 size = bWillDrawBottom ? ImVec2{ 0, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight() - ImGui::GetFramePadding().y * 3 } : ImVec2{ 0, 0 };
		if (ImGui::BeginChild("AssetBrowserMainWindow", size))
		{
			DrawLeftColumn();

			static constexpr float spacing = 3.0f;

			ImGui::SameLine(0.0f, spacing);

			DrawColumnSplitter(contentWidth);

			ImGui::SameLine(0.0f, spacing);

			DrawRightColumn();
		}

		ImGui::EndChild();

		if (bWillDrawBottom)
		{
			ImGui::Separator();

			DrawBottom();
		}
	}

	void AssetBrowserPanelBase::ProcessEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) { return OnKeyPressed(e); });
	}

	bool AssetBrowserPanelBase::OnKeyPressed(KeyPressedEvent& e)
	{
		if (!IsPanelFocused()) return false;

		switch (e.GetKeyCode())
		{
			case Key::Enter:	RequestPathOpen(m_SelectedPath);		return true;
			case Key::F2:		RequestPathRenaming(m_SelectedPath);	return true;
			case Key::Delete:	RequestPathDeletion(m_SelectedPath);	return true;
		}

		return false;
	}

	void AssetBrowserPanelBase::DrawTopBar()
	{
		// ViewType menu
		if (ImGui::BeginPopupWithPadding("View Type"))
		{
			if (ImGui::RadioButton("Tiles", m_ViewType == AssetBrowserViewType::Tiles))
			{
				m_ViewType = AssetBrowserViewType::Tiles;
			}
			if (ImGui::RadioButton("List", m_ViewType == AssetBrowserViewType::List))
			{
				m_ViewType = AssetBrowserViewType::List;
			}

			ImGui::EndPopup();
		}
		if (ImGui::Button(ICON_FA_EYE " View Type"))
		{
			ImGui::OpenPopup("View Type");
		}

		ImGui::SameLine();

		m_Filter.Draw("##AssetBrowserFilter", "Search assets");
	}

	void AssetBrowserPanelBase::DrawLeftColumn()
	{
		if (ImGui::BeginChild("AssetBrowserLeftColumn", ImVec2(m_LeftColumnWidth, 0)))
		{
			DrawDirectoryTree();
		}

		ImGui::EndChild();
	}

	void AssetBrowserPanelBase::DrawDirectoryTree()
	{
		m_LeftColumnWindowId = ImGui::GetCurrentWindow()->ID;

		const std::string projectAssetDir = AssetRegistry::GetProjectPathPrefix();
		ImGuiTreeNodeFlags flags = (m_SelectedDirectory == projectAssetDir ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
		const auto directoryMetadata = AssetRegistry::Get().GetPathMetadata<DirectoryMetadata>(projectAssetDir);
		const char* rootDirectoryName = directoryMetadata->bIsTreeExpanded ? ICON_FA_FOLDER_OPEN " Assets###Assets" : ICON_FA_FOLDER " Assets###Assets";
		directoryMetadata->bIsTreeExpanded = ImGui::TreeNodeEx(rootDirectoryName, flags);
		directoryMetadata->TreeNodeId = ImGui::GetCurrentWindow()->GetID(rootDirectoryName);
		if (ImGui::IsItemClicked())
		{
			m_SelectedDirectory = projectAssetDir;
		}
		if (directoryMetadata->bIsTreeExpanded)
		{
			DrawDirectoryTreeRecursively(projectAssetDir);

			ImGui::TreePop();
		}
	}

	void AssetBrowserPanelBase::DrawDirectoryTreeRecursively(const std::string& baseDirectory)
	{
		AssetRegistry& ar = AssetRegistry::Get();
		ar.ForEachPathInDirectory(baseDirectory, [this, &ar](const std::string& path)
		{
			const auto directoryMetadata = ar.GetPathMetadata<DirectoryMetadata>(path);
			if (!directoryMetadata) return;

			char directoryName[MAX_PATH_SIZE] = ICON_FA_FOLDER " ";
			if (directoryMetadata->bIsTreeExpanded)
			{
				strcpy_s(directoryName, ICON_FA_FOLDER_OPEN " ");
			}
			const char* name = directoryMetadata->PathName.c_str();
			strcat_s(directoryName, name);
			strcat_s(directoryName, "###");
			strcat_s(directoryName, name);
			ImGuiTreeNodeFlags flags = (m_SelectedDirectory == path ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
			if (!directoryMetadata->bHasAnySubDirectory)
			{
				flags |= ImGuiTreeNodeFlags_Leaf;
			}
			directoryMetadata->bIsTreeExpanded = ImGui::TreeNodeEx(directoryName, flags);
			directoryMetadata->TreeNodeId = ImGui::GetCurrentWindow()->GetID(directoryName);
			if (ImGui::IsItemClicked())
			{
				m_SelectedDirectory = path;
			}
			if (directoryMetadata->bIsTreeExpanded)
			{
				DrawDirectoryTreeRecursively(path);

				ImGui::TreePop();
			}
			// Do not show folder_open icon if this is a leaf directory
			if (!directoryMetadata->bHasAnySubDirectory)
			{
				directoryMetadata->bIsTreeExpanded = false;
			}
		});
	}

	void AssetBrowserPanelBase::DrawColumnSplitter(float contentWidth)
	{
		ImVec2 leftColumnSize(m_LeftColumnWidth, 0);
		ImGui::VSplitter("AssetBrowserVSplitter", &leftColumnSize);
		static constexpr float columnPadding = 15.0f;
		m_LeftColumnWidth = glm::clamp(leftColumnSize.x, columnPadding, contentWidth - columnPadding);
	}

	void AssetBrowserPanelBase::DrawRightColumn()
	{
		if (ImGui::BeginChild("AssetBrowserRightColumn"))
		{
			DrawDirectoryNavigator();

			ImGui::Separator();

			if (IsAnyFilterActive())
			{
				DrawFilteredAssetsInDirectoryRecursively();
			}
			else
			{
				DrawPathsInDirectory();
			}

			// Deselect path when blank space is clicked
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
			{
				m_SelectedPath.clear();
			}

			DrawWindowContextMenu();

			if (!m_PathToDelete.empty())
			{
				ProcessPathDeletion(m_PathToDelete);
			}
		}

		ImGui::EndChild();
	}

	void AssetBrowserPanelBase::DrawDirectoryNavigator()
	{
		I32 i = 0;
		for (const auto& parentFolder : EngineUtils::SplitString(m_SelectedDirectory, '/'))
		{
			ImGui::SameLine(0.0f, 0.0f);
			if (ImGui::TransparentSmallButton(parentFolder.c_str()))
			{
				std::string jumpToDirectory;
				I32 j = 0;
				for (const auto& parentFolder2 : EngineUtils::SplitString(m_SelectedDirectory, '/'))
				{
					if (jumpToDirectory.empty())
					{
						jumpToDirectory = parentFolder2;
					}
					else
					{
						jumpToDirectory = fmt::format("{}/{}", jumpToDirectory, parentFolder2);
					}

					if (j++ == i) break;
				}
				m_SelectedDirectory = std::move(jumpToDirectory);
				break;
			}
			ImGui::SameLine(0.0f, 0.0f);
			ImGui::Text(ICON_FA_ARROW_RIGHT);

			++i;
		}
	}

	void AssetBrowserPanelBase::DrawPathsInDirectory()
	{
		const auto& paths = AssetRegistry::Get().GetPathsInDirectory(m_SelectedDirectory);
		bool bHasDrawnAnyPath = false;
		if (!paths.empty())
		{
			if (m_ViewType == AssetBrowserViewType::Tiles)
			{
				static constexpr float padding = 16.0f;
				static const float thumbnailWidth = GetTileThumbnailWidth();
				const float cellSize = thumbnailWidth + padding;
				const float panelWidth = ImGui::GetContentRegionAvail().x;
				I32 columnCount = static_cast<I32>(panelWidth / cellSize);
				if (columnCount < 1)
				{
					columnCount = 1;
				}
				if (ImGui::BeginTable("", columnCount))
				{
					const float tableOffset = GetTableOffset();
					ImGui::Indent(tableOffset);
					for (const auto& path : paths)
					{
						const auto metadata = AssetRegistry::Get().GetPathMetadata(path);
						if (ShouldDrawPath(metadata))
						{
							ImGui::TableNextColumn();

							DrawTilePath(metadata);
							bHasDrawnAnyPath = true;
						}
					}
					ImGui::Unindent(tableOffset);

					ImGui::EndTable();
				}
			}
			else
			{
				ImGuiListClipper clipper;
				clipper.Begin(static_cast<I32>(paths.size()), GetSelectableThumbnailWidth());
				const auto it = paths.begin();
				if (m_bFocusSelectedPath)
				{
					for (SizeT i = 0; i < paths.size(); ++i)
					{
						if (paths[i] == m_SelectedPath)
						{
							// First bring selected path into view, later it will get focused
							// BUG: For now if selected path is the last one in the current directory, it will not get focused properly
							clipper.ForceDisplayRangeByIndices(static_cast<int>(i), static_cast<int>(i + 1));
							break;
						}
					}
				}
				while (clipper.Step())
				{
					for (auto index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
					{
						const auto metadata = AssetRegistry::Get().GetPathMetadata(*(it + index));
						if (ShouldDrawPath(metadata))
						{
							DrawSelectablePath(metadata);
							bHasDrawnAnyPath = true;
						}
					}
				}
			}
		}
		if (!bHasDrawnAnyPath)
		{
			m_Filter.DrawEmptyText();
		}

		if (!m_DirectoryToOpen.empty())
		{
			// Double-click to open directory in the right column
			HandleRightColumnDirectoryOpen(m_DirectoryToOpen);
			m_DirectoryToOpen.clear();
		}
	}

	void AssetBrowserPanelBase::DrawFilteredAssetsInDirectoryRecursively()
	{
		// Only update cache when search filter or type filter changes
		if (ShouldUpdateFilterCache())
		{
			m_FilteredPaths.clear();

			AssetRegistry::Get().ForEachPathInDirectoryRecursively(m_SelectedDirectory, [this](const std::string& path)
			{
				const auto metadata = AssetRegistry::Get().GetPathMetadata(path);
				if (!metadata->IsAsset()) return;

				if (m_Filter.PassFilter(metadata->PathName.c_str()))
				{
					if (PassFilter(metadata))
					{
						m_FilteredPaths.emplace_back(path);
					}
				}
			});

			m_bForceUpdateFilterCache = false;
		}

		if (m_ViewType == AssetBrowserViewType::Tiles)
		{
			static constexpr float padding = 16.0f;
			static const float thumbnailWidth = GetTileThumbnailWidth();
			const float cellSize = thumbnailWidth + padding;
			const float panelWidth = ImGui::GetContentRegionAvail().x;
			I32 columnCount = static_cast<I32>(panelWidth / cellSize);
			if (columnCount < 1)
			{
				columnCount = 1;
			}
			if (ImGui::BeginTable("", columnCount))
			{
				const float tableOffset = GetTableOffset();
				ImGui::Indent(tableOffset);
				for (const auto& filteredPath : m_FilteredPaths)
				{
					const auto metadata = AssetRegistry::Get().GetPathMetadata(filteredPath);
					if (ShouldDrawPath(metadata))
					{
						ImGui::TableNextColumn();

						DrawTilePath(metadata);
					}
				}
				ImGui::Unindent(tableOffset);

				ImGui::EndTable();
			}
		}
		else
		{
			ImGuiListClipper clipper;
			clipper.Begin(static_cast<I32>(m_FilteredPaths.size()), GetSelectableThumbnailWidth());
			const auto it = m_FilteredPaths.begin();
			while (clipper.Step())
			{
				for (auto index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
				{
					const auto metadata = AssetRegistry::Get().GetPathMetadata(*(it + index));
					if (ShouldDrawPath(metadata))
					{
						DrawSelectablePath(metadata);
					}
				}
			}
		}

		if (m_FilteredPaths.empty())
		{
			m_Filter.DrawEmptyText();
		}
	}

	void AssetBrowserPanelBase::DrawWindowContextMenu()
	{
		// Right-click on blank space
		if (ImGui::BeginPopupContextWindowWithPadding(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			// Actual directory or asset is created after renaming

			const float thumbnailWidth = GetSelectableThumbnailWidth();

			bool bIsFolderCreationSelected = ImGui::Selectable("##FolderCreationSelectable", false, 0, ImVec2(0.0f, thumbnailWidth));

			ImGui::SameLine();

			// Draw folder icon
			ImGui::Image(ThumbnailManager::Get().GetDirectoryIcon()->GetTextureID(),
				{ thumbnailWidth, thumbnailWidth },
				{ 0.0f, 1.0f }, { 1.0f, 0.0f });

			ImGui::SameLine();

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Create New Folder");

			if (bIsFolderCreationSelected)
			{
				const auto newPath = GetAvailableNewPathName("NewFolder", false);
				RequestPathCreation(newPath, {}, true);
			}

			DrawWindowContextMenuImpl(thumbnailWidth);

			ImGui::EndPopup();
		}
	}

	void AssetBrowserPanelBase::ClearAllFilters()
	{
		m_Filter.Clear();
	}

	std::string AssetBrowserPanelBase::GetFormattedAssetTypeName(const char* typeName) const
	{
		if (strlen(typeName) == 0) return {};

		std::string typeNameStr = typeName;

		// Format certain letters to be upper case
		typeNameStr[0] = std::toupper(typeNameStr[0]);
		for (SizeT i = 0; i < typeNameStr.size(); ++i)
		{
			if (typeNameStr[i] == ' ' && i + 1 < typeNameStr.size())
			{
				typeNameStr[i + 1] = std::toupper(typeNameStr[i + 1]);
			}
		}

		// Remove all spaces
		typeNameStr.erase(std::remove_if(typeNameStr.begin(), typeNameStr.end(),
			[](unsigned char c) { return std::isspace(c); }), // std::isspace cannot be used directly here, see "Notes" part in https://en.cppreference.com/w/cpp/string/byte/isspace
			typeNameStr.end());

		return typeNameStr;
	}

	std::string AssetBrowserPanelBase::GetAvailableNewPathName(const char* baseName, bool bIsAsset) const
	{
		I32 i = 0;
		char suffix[10];
		char newName[MAX_PATH_SIZE];
		strcpy_s(newName, baseName);
		if (bIsAsset)
		{
			strcat_s(newName, AssetRegistry::GetEngineAssetExtension());
		}
		auto newPath = fmt::format("{}/{}", m_SelectedDirectory, newName);
		while (AssetRegistry::Get().ContainsPathInDirectory(m_SelectedDirectory, newPath))
		{
			_itoa_s(++i, suffix, 10);
			strcpy_s(newName, baseName);
			strcat_s(newName, suffix);
			if (bIsAsset)
			{
				strcat_s(newName, AssetRegistry::GetEngineAssetExtension());
			}
			newPath = fmt::format("{}/{}", m_SelectedDirectory, newName);
		}
		return newPath;
	}

	void AssetBrowserPanelBase::DrawSelectablePath(const Ref<PathMetadata>& metadata)
	{
		const auto& path = metadata->Path;
		// Push path as id
		ImGui::PushID(path.c_str());
		{
			const bool bIsAsset = metadata->IsAsset();
			const bool bPathNeedsRenaming = m_PathToRename == path;
			static const float thumbnailWidth = GetSelectableThumbnailWidth();
			static constexpr float thumbnailRounding = 4.0f;
			const auto pathName = metadata->PathName.c_str();

			ImGuiSelectableFlags flags = bPathNeedsRenaming ? ImGuiSelectableFlags_Disabled : 0; // Disable selectable during renaming so that text can be selected
			flags |= ImGuiSelectableFlags_AllowItemOverlap;
			const bool bIsSelectedPath = m_SelectedPath == path;
			const bool bIsSelected = ImGui::Selectable("##PathSelectable", bIsSelectedPath, flags, { 0.0f, thumbnailWidth });

			if (bIsSelectedPath && m_bFocusSelectedPath)
			{
				ImGui::SetScrollHereY(1.0f);
				m_bFocusSelectedPath = false;
			}

			// Draw path tooltip on hover
			DrawPathTooltip(metadata);

			// Draw path context menu on right click
			DrawPathContextMenu(path);

			// Process asset dragging
			ProcessAssetDragging(metadata, thumbnailRounding);

			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
			{
				if (bIsAsset)
				{
					// Double-click to open asset editor
					HandleRightColumnAssetOpen(path);
				}
				else
				{
					// Delay opening directory as clipper's iteration has not finished yet
					m_DirectoryToOpen = path;
				}
			}

			ImGui::SameLine();

			// Draw selectable thumbnail
			ImGui::AssetThumbnail(metadata->ThumbnailTexture->GetTextureID(),
				thumbnailWidth, thumbnailRounding, false);

			ImGui::SameLine();

			// Draw path name or rename box
			ImGui::BeginGroup();
			{
				if (!bPathNeedsRenaming)
				{
					// Make two lines of text more compact
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
					// Display directory/asset name
					ImGui::Text(pathName);
					// Display type name
					ImGui::TextColored({ 0.6f, 0.6f, 0.6f, 1.0f }, bIsAsset ? AssetManager::Get().GetAssetFactoryByAssetType(metadata->GetAssetTypeID())->GetAssetTypeName() : "Folder");
					ImGui::PopStyleVar();
				}
				else
				{
					// Clear selection as long as renaming is in process
					m_SelectedPath.clear();

					static bool bHasKeyboardFocused = false;
					if (!bHasKeyboardFocused)
					{
						// Focus input once
						ImGui::SetKeyboardFocusHere();
						bHasKeyboardFocused = true;
					}
					ImGui::SetNextItemWidth(ImGui::CalcTextSize(pathName).x + ImGui::GetFramePadding().x * 2.0f);
					char renameBuffer[MAX_PATH_SIZE];
					strcpy_s(renameBuffer, pathName);
					ImGui::InputText("##RenamePath", renameBuffer, sizeof(renameBuffer), ImGuiInputTextFlags_AutoSelectAll);
					SubmitPathRenaming(renameBuffer, metadata, bHasKeyboardFocused);
				}
			}
			ImGui::EndGroup();

			if (bIsSelected)
			{
				m_SelectedPath = path;
				OnPathSelected(path);
			}
		}
		ImGui::PopID();
	}

	void AssetBrowserPanelBase::DrawTilePath(const Ref<PathMetadata>& metadata)
	{
		const auto& path = metadata->Path;
		// Push path as id
		ImGui::PushID(path.c_str());
		{
			const bool bIsAsset = metadata->IsAsset();
			const bool bPathNeedsRenaming = m_PathToRename == path;
			static const float thumbnailWidth = GetTileThumbnailWidth();
			static constexpr float thumbnailRounding = 4.0f;
			const auto pathName = metadata->PathName.c_str();

			const bool bIsSelectedPath = m_SelectedPath == path;
			ImGui::TileImageButton(metadata->ThumbnailTexture->GetTextureID(), bIsAsset, bPathNeedsRenaming, // Disable button during renaming so that text can be selected
				bIsSelectedPath,
				{ thumbnailWidth, thumbnailWidth }, thumbnailRounding,
				{ 0.0f, 1.0f }, { 1.0f, 0.0f });

			// Draw path tooltip on hover
			DrawPathTooltip(metadata);

			// Draw path context menu on right click
			DrawPathContextMenu(path);

			// Process asset dragging
			ProcessAssetDragging(metadata, thumbnailRounding);

			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
			{
				if (bIsAsset)
				{
					// Double-click to open asset editor
					HandleRightColumnAssetOpen(path);
				}
				else
				{
					// Delay opening directory as clipper's iteration has not finished yet
					m_DirectoryToOpen = path;
				}
			}
			else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
			{
				m_SelectedPath = path;
				OnPathSelected(path);
			}

			const float textSize = ImGui::CalcTextSize(pathName).x;
			if (!bPathNeedsRenaming)
			{
				// Display directory/asset name
				const float indent = (thumbnailWidth - textSize) * 0.5f + ImGui::GetFramePadding().x;
				if (indent > 0.0f)
				{
					// Center the wrapped text
					ImGui::Indent(indent);
				}
				ImGui::PushClipRect(ImGui::GetItemRectMin(), ImVec2{ ImGui::GetItemRectMax().x, ImGui::GetItemRectMax().y - ImGui::GetFramePadding().y }, true);
				ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + thumbnailWidth);
				ImGui::TextUnformatted(pathName);
				ImGui::PopTextWrapPos();
				ImGui::PopClipRect();
				if (indent > 0.0f)
				{
					ImGui::Unindent(indent);
				}
			}
			else
			{
				// Used to allow the following input text to receive hovered event
				ImGui::SetItemAllowOverlap();

				// Clear selection as long as renaming is in process
				m_SelectedPath.clear();

				static bool bHasKeyboardFocused = false;
				if (!bHasKeyboardFocused)
				{
					// Focus input once
					ImGui::SetKeyboardFocusHere();
					bHasKeyboardFocused = true;
				}

				char renameBuffer[MAX_PATH_SIZE];
				strcpy_s(renameBuffer, pathName);
				const float indent = (thumbnailWidth - textSize) * 0.5f;
				if (indent > 0.0f)
				{
					ImGui::Indent(indent);
				}
				ImGui::SetNextItemWidth(textSize < thumbnailWidth ? textSize + ImGui::GetFramePadding().x * 2 : thumbnailWidth);
				ImGui::InputText("##RenamePath", renameBuffer, sizeof(renameBuffer), ImGuiInputTextFlags_AutoSelectAll);
				if (indent > 0.0f)
				{
					ImGui::Unindent(indent);
				}
				SubmitPathRenaming(renameBuffer, metadata, bHasKeyboardFocused);
			}

			if (bIsSelectedPath && m_bFocusSelectedPath)
			{
				ImGui::SetScrollHereY(1.0f);
				m_bFocusSelectedPath = false;
			}
		}
		ImGui::PopID();
	}

	// TODO: Add path tooltip implementation to AssetActions
	void AssetBrowserPanelBase::DrawPathTooltip(const Ref<PathMetadata>& metadata) const
	{
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltipWithPadding();
			ImGui::Text(metadata->PathName.c_str());
			ImGui::Separator();
			ImGui::Text("Type: %s", metadata->IsAsset() ? AssetManager::Get().GetAssetFactoryByAssetType(metadata->GetAssetTypeID())->GetAssetTypeName() : "Folder");
			ImGui::Text("Path: %s", metadata->Path.c_str());
			if (const auto assetMetadata = std::dynamic_pointer_cast<AssetMetadata>(metadata))
			{
				if (assetMetadata->IsImportableAsset())
				{
					ImGui::Text("Source Path: %s", assetMetadata->SourcePath.c_str());
				}
			}
			ImGui::EndTooltipWithPadding();
		}
	}

	void AssetBrowserPanelBase::DrawPathContextMenu(const std::string& path)
	{
		if (ImGui::BeginPopupContextItemWithPadding(nullptr))
		{
			m_SelectedPath = path;

			const auto metadata = AssetRegistry::Get().GetPathMetadata(path);
			const bool bIsAsset = metadata->IsAsset();

			if (ImGui::MenuItem("Rename", "F2"))
			{
				RequestPathRenaming(path);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Rename this file");
			}

			DrawPathContextMenuItem_Save(path, bIsAsset);

			if (ImGui::MenuItem("Delete", "Delete"))
			{
				RequestPathDeletion(path);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Delete this file");
			}

			ImGui::Separator();

			if (bIsAsset)
			{
				DrawPathContextMenuItem_Asset(path, std::static_pointer_cast<AssetMetadata>(metadata));
			}

			if (ImGui::MenuItem("Show In Explorer"))
			{
				PlatformUtils::ShowInExplorer(FileSystemUtils::GetFileSystemPath(path));
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Show in external file explorer");
			}

			ImGui::EndPopup();
		}
	}

	void AssetBrowserPanelBase::SubmitPathRenaming(char* renameBuffer, const Ref<PathMetadata>& metadata, bool& bHasKeyboardFocused)
	{
		// ImGui::IsItemDeactivated() will not get called for right-click, so we add these specific check
		if (ImGui::IsItemDeactivated() || (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !ImGui::IsItemHovered()))
		{
			if (metadata->IsAsset())
			{
				// Add engine file extension automatically
				strcat_s(renameBuffer, MAX_PATH_SIZE, AssetRegistry::GetEngineAssetExtension());
			}
			const auto& path = metadata->Path;
			const auto parentPath = FileSystemUtils::GetParentPath(path);
			auto newPath = fmt::format("{}/{}", parentPath, renameBuffer);
			if (newPath != path && AssetRegistry::Get().ContainsPathInDirectory(parentPath, newPath))
			{
				ZE_CORE_WARN("Failed to rename {0} to {1}! Path already exist.", path, newPath);
				newPath = path;
			}
			ProcessPathRenaming(path, newPath, metadata->GetAssetTypeID());
			bHasKeyboardFocused = false;
			m_bForceUpdateFilterCache = true;
		}
	}

	void AssetBrowserPanelBase::HandleRightColumnDirectoryOpen(const std::string& directory)
	{
		m_SelectedDirectory = directory;

		ImGuiStorage* storage = ImGui::FindWindowByID(m_LeftColumnWindowId)->DC.StateStorage;
		// Manually toggle upper-level tree node open iteratively
		auto currentDirectory = directory;
		const auto& ar = AssetRegistry::Get();
		while (currentDirectory != AssetRegistry::GetProjectPathPrefix())
		{
			storage->SetInt(ar.GetPathMetadata<DirectoryMetadata>(currentDirectory)->TreeNodeId, true);
			currentDirectory = FileSystemUtils::GetParentPath(currentDirectory);
		}
		storage->SetInt(ar.GetPathMetadata<DirectoryMetadata>(currentDirectory)->TreeNodeId, true);

		// Clear at last to prevent crash!
		m_SelectedPath.clear();
	}

	void AssetBrowserPanelBase::RequestPathCreation(const std::string& path, AssetTypeID typeID, bool bNeedsRenaming)
	{
		if (bNeedsRenaming)
		{
			m_PathToRename = path;
			m_PathToCreate = path;
			AssetRegistry::Get().OnTempPathCreated(path, typeID);
		}
		else
		{
			if (AssetManager::Get().CreateAssetFile(typeID, path))
			{
				AssetRegistry::Get().OnPathCreated(path, typeID);
			}
		}
		ClearAllFilters(); // Keep filters active during path creation is meaningless
	}

	void AssetBrowserPanelBase::RequestPathCreationForResourceAsset(const std::string& srcPath, const std::string& destPath)
	{
		FileSystemUtils::CopyAsset(srcPath, destPath);
		AssetRegistry::Get().OnPathCreated(destPath, true);
		ClearAllFilters(); // Keep filters active during path creation is meaningless
	}

	void AssetBrowserPanelBase::RequestPathDeletion(const std::string& path)
	{
		if (!path.empty())
		{
			m_PathToDelete = path;
		}
	}

	void AssetBrowserPanelBase::RequestPathRenaming(const std::string& path)
	{
		if (!path.empty())
		{
			m_PathToRename = path;
		}
	}

	void AssetBrowserPanelBase::RequestPathOpen(const std::string& path)
	{
		if (!path.empty())
		{
			const auto metadata = AssetRegistry::Get().GetPathMetadata(path);
			if (metadata->IsAsset())
			{
				HandleRightColumnAssetOpen(path);
			}
			else
			{
				HandleRightColumnDirectoryOpen(path);
			}
		}
	}

	void AssetBrowserPanelBase::ProcessPathDeletion(const std::string& path)
	{
		static ImVec2 buttonSize{ 120.0f, 0.0f };
		ImGui::OpenPopup("Delete?");
		// Deletion request dialog
		if (ImGui::BeginPopupModal("Delete?", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::Text("The selected folder/asset will be permanently deleted.\nThis operation cannot be undone!\n\n");

			if (ImGui::Button("OK", buttonSize))
			{
				const auto metadata = AssetRegistry::Get().GetPathMetadata(path);
				if (!metadata->IsAsset())
				{
					// Delete a directory
					FileSystemUtils::DeletePath(path);
					AssetRegistry::Get().OnPathRemoved(path);
				}
				else
				{
					// Delete an asset
					AssetManager::Get().DeleteAsset(path);
				}
				// Clear current selection
				m_SelectedPath.clear();
				m_PathToDelete.clear();
				m_bForceUpdateFilterCache = true;

				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();

			const float availableWidth = ImGui::GetContentRegionAvail().x;
			ImGui::SameLine(availableWidth - buttonSize.x);

			if (ImGui::Button("Cancel", buttonSize))
			{
				m_PathToDelete.clear();

				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void AssetBrowserPanelBase::ProcessPathRenaming(const std::string& oldPath, const std::string& newPath, AssetTypeID typeID)
	{
		if (m_PathToCreate.empty())
		{
			if (newPath != oldPath)
			{
				// Regular renaming process
				if (typeID)
				{
					AssetManager::Get().RenameAsset(oldPath, newPath);
				}
				else
				{
					FileSystemUtils::RenamePath(oldPath, newPath);
				}
			}
		}
		else
		{
			// Triggered by path creation
			if (typeID)
			{
				AssetManager::Get().CreateAssetFile(typeID, newPath);
			}
			else
			{
				FileSystemUtils::CreateDirectory(newPath);
			}
			m_PathToCreate.clear();
		}
		m_SelectedPath = newPath;
		if (newPath != oldPath)
		{
			AssetRegistry::Get().OnPathRenamed(oldPath, newPath);
		}
		m_PathToRename.clear();
	}

}
