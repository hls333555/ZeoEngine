#include "Panels/ContentBrowserPanel.h"

#include <IconsFontAwesome5.h>

#include "Engine/Debug/BenchmarkTimer.h"
#include "Engine/Core/Assert.h"
#include "Engine/Utils/PathUtils.h"
#include "Utils/EditorUtils.h"
#include "Engine/Core/AssetManager.h"
#include "Engine/Core/AssetFactory.h"
#include "Engine/Core/AssetActions.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/Core/ThumbnailManager.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Utils/PlatformUtils.h"

#define MAX_PATH_SIZE 128

namespace ZeoEngine {

	namespace Utils {
	
		static std::string GetFormatedAssetTypeName(const char* typeName)
		{
			if (strlen(typeName) == 0) return {};

			std::string typeNameStr = typeName;

			// Format certain letters to be upper case
			typeNameStr[0] = std::toupper(typeNameStr[0]);
			for (size_t i = 0; i < typeNameStr.size(); ++i)
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

		static float GetSelectableThumbnailWidth()
		{
			return ImGui::GetStyle().Alpha * 32.0f;
		}

		static float GetTileThumbnailWidth()
		{
			return ImGui::GetStyle().Alpha * 64.0f;
		}

	}

	void ContentBrowserPanel::OnAttach()
	{
		m_SelectedDirectory = AssetRegistry::GetAssetRootDirectory();

		InitAssetTypeFilters();
	}

	void ContentBrowserPanel::ProcessRender()
	{
		DrawTopBar();

		ImGui::Separator();

		float contentWidth = ImGui::GetContentRegionAvail().x;

		DrawLeftColumn();

		static constexpr float spacing = 3.0f;

		ImGui::SameLine(0.0f, spacing);

		DrawColumnSplitter(contentWidth);

		ImGui::SameLine(0.0f, spacing);

		DrawRightColumn();
	}

	void ContentBrowserPanel::ProcessEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ZE_BIND_EVENT_FUNC(ContentBrowserPanel::OnKeyPressed));
	}

	bool ContentBrowserPanel::OnKeyPressed(KeyPressedEvent& e)
	{
		if (!IsPanelFocused()) return false;

		switch (e.GetKeyCode())
		{
			case Key::Enter:
				RequestPathOpen(m_SelectedPath);
				break;
			case Key::F2:
				RequestPathRenaming(m_SelectedPath);
				break;
			case Key::Delete:
				RequestPathDeletion(m_SelectedPath);
				break;
			default:
				break;
		}

		return false;
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
				std::string destPath = PathUtils::AppendPath(m_SelectedDirectory, assetName);
				auto& am = AssetManager::Get();
				am.ImportAsset(*am.GetTypdIdFromFileExtension(extension), PathUtils::GetRelativePath(*filePath), destPath);
				m_bShouldUpdateFilterCache = true;
			}
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltipWithPadding("Import to %s", m_SelectedDirectory.c_str());
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

		// ViewType menu
		if (ImGui::BeginPopupWithPadding("View Type"))
		{
			if (ImGui::RadioButton("Tiles", m_ViewType == ContentBrowserViewType::Tiles))
			{
				m_ViewType = ContentBrowserViewType::Tiles;
			}
			if (ImGui::RadioButton("List", m_ViewType == ContentBrowserViewType::List))
			{
				m_ViewType = ContentBrowserViewType::List;
			}

			ImGui::EndPopup();
		}
		if (ImGui::Button(ICON_FA_EYE " View Type"))
		{
			ImGui::OpenPopup("View Type");
		}

		ImGui::SameLine();

		m_Filter.Draw("##ContentBrowserFilter", "Search assets", -1.0f);
	}

	void ContentBrowserPanel::DrawLeftColumn()
	{
		if (ImGui::BeginChild("ContentBrowserLeftColumn", ImVec2(m_LeftColumnWidth, 0.0f)))
		{
			DrawDirectoryTree();
		}

		ImGui::EndChild();
	}

	void ContentBrowserPanel::DrawDirectoryTree()
	{
		m_LeftColumnWindowId = ImGui::GetCurrentWindow()->ID;

		ImGuiTreeNodeFlags flags = (m_SelectedDirectory == AssetRegistry::GetAssetRootDirectory() ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
		auto directorySpec = AssetRegistry::Get().GetPathSpec<DirectorySpec>(AssetRegistry::GetAssetRootDirectory());
		const char* rootDirectoryName = directorySpec->bIsTreeExpanded ? ICON_FA_FOLDER_OPEN " Assets###Assets" : ICON_FA_FOLDER " Assets###Assets";
		directorySpec->bIsTreeExpanded = ImGui::TreeNodeEx(rootDirectoryName, flags);
		directorySpec->TreeNodeId = ImGui::GetCurrentWindow()->GetID(rootDirectoryName);
		if (ImGui::IsItemClicked())
		{
			m_SelectedDirectory = AssetRegistry::GetAssetRootDirectory();
		}
		if (directorySpec->bIsTreeExpanded)
		{
			//BEGIN_BENCHMARK()

			DrawDirectoryTreeRecursively(AssetRegistry::GetAssetRootDirectory());

			//END_BENCHMARK()

			ImGui::TreePop();
		}
	}

	void ContentBrowserPanel::DrawDirectoryTreeRecursively(const std::string& baseDirectory)
	{
		AssetRegistry& ar = AssetRegistry::Get();
		ar.ForEachPathInDirectory(baseDirectory, [this, &ar](const std::string& path)
		{
			auto directorySpec = ar.GetPathSpec<DirectorySpec>(path);
			if (!directorySpec) return;

			char directoryName[MAX_PATH_SIZE] = ICON_FA_FOLDER " ";
			if (directorySpec->bIsTreeExpanded)
			{
				strcpy_s(directoryName, ICON_FA_FOLDER_OPEN " ");
			}
			const char* name = directorySpec->PathName.c_str();
			strcat_s(directoryName, name);
			strcat_s(directoryName, "###");
			strcat_s(directoryName, name);
			ImGuiTreeNodeFlags flags = (m_SelectedDirectory == path ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
			if (!directorySpec->bHasAnySubDirectory)
			{
				flags |= ImGuiTreeNodeFlags_Leaf;
			}
			directorySpec->bIsTreeExpanded = ImGui::TreeNodeEx(directoryName, flags);
			directorySpec->TreeNodeId = ImGui::GetCurrentWindow()->GetID(directoryName);
			if (ImGui::IsItemClicked())
			{
				m_SelectedDirectory = path;
			}
			if (directorySpec->bIsTreeExpanded)
			{
				DrawDirectoryTreeRecursively(path);

				ImGui::TreePop();
			}
			// Do not show folder_open icon if this is a leaf directory
			if (!directorySpec->bHasAnySubDirectory)
			{
				directorySpec->bIsTreeExpanded = false;
			}
		});
	}

	void ContentBrowserPanel::DrawColumnSplitter(float contentWidth)
	{
		ImVec2 leftColumnSize(m_LeftColumnWidth, 0.0f);
		ImGui::VSplitter("ContentBrowserVSplitter", &leftColumnSize);
		static constexpr float columnPadding = 15.0f;
		m_LeftColumnWidth = glm::clamp(leftColumnSize.x, columnPadding, contentWidth - columnPadding);
	}

	void ContentBrowserPanel::DrawRightColumn()
	{
		if (ImGui::BeginChild("ContentBrowserRightColumn"))
		{
			DrawDirectoryNavigator();

			ImGui::Separator();

			if (!m_Filter.IsActive() && !m_bIsAnyTypeFilterActive)
			{
				DrawPathsInDirectory();
			}
			else
			{
				DrawFilteredAssetsInDirectoryRecursively();
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

	void ContentBrowserPanel::DrawDirectoryNavigator()
	{
		int32_t i = 0;
		PathUtils::ForEachParentPathInPath(m_SelectedDirectory, [this, &i](const std::string& name)
		{
			ImGui::SameLine(0.0f, 0.0f);
			if (ImGui::TransparentSmallButton(name.c_str()))
			{
				std::string jumpToDirectory;
				int32_t j = 0;
				PathUtils::ForEachParentPathInPath(m_SelectedDirectory, [&jumpToDirectory, i, &j](const std::string& name)
				{
					jumpToDirectory = PathUtils::AppendPath(jumpToDirectory, name);
					if (j++ == i) return true;
					return false;
				});
				m_SelectedDirectory = std::move(jumpToDirectory);
			}
			ImGui::SameLine(0.0f, 0.0f);
			ImGui::Text(ICON_FA_ARROW_RIGHT);

			++i;
			return false;
		});
	}

	void ContentBrowserPanel::DrawPathsInDirectory()
	{
		const auto& paths = AssetRegistry::Get().GetPathsInDirectory(m_SelectedDirectory);
		if (paths.size() > 0)
		{
			if (m_ViewType == ContentBrowserViewType::Tiles)
			{
				static constexpr float padding = 16.0f;
				static const float thumbnailWidth = Utils::GetTileThumbnailWidth();
				const float cellSize = thumbnailWidth + padding;
				const float panelWidth = ImGui::GetContentRegionAvail().x;
				int32_t columnCount = static_cast<int32_t>(panelWidth / cellSize);
				if (columnCount < 1)
				{
					columnCount = 1;
				}
				if (ImGui::BeginTable("", columnCount))
				{
					for (auto it = paths.begin(); it != paths.end(); ++it)
					{
						ImGui::TableNextColumn();

						DrawTilePath(*it);
					}

					ImGui::EndTable();
				}
			}
			else
			{
				ImGuiListClipper clipper;
				clipper.Begin(static_cast<int32_t>(paths.size()), Utils::GetSelectableThumbnailWidth());
				auto it = paths.begin();
				while (clipper.Step())
				{
					for (auto index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
					{
						DrawSelectablePath(*(it + index));
					}
				}
			}
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			ImGui::TextCentered("This folder is empty");
			ImGui::PopStyleColor();
		}

		if (!m_DirectoryToOpen.empty())
		{
			m_SelectedDirectory = m_DirectoryToOpen;
			// Double-click to open directory in the right column
			HandleRightColumnDirectoryOpen(m_DirectoryToOpen);
			m_DirectoryToOpen.clear();
		}
	}

	void ContentBrowserPanel::DrawFilteredAssetsInDirectoryRecursively()
	{
		//BEGIN_BENCHMARK()

		// Only update cache when search filter or type filter changes
		if (m_Filter.bIsInputBufferChanged || m_bIsTypeFilterChanged || m_bShouldUpdateFilterCache)
		{
			m_FilteredPaths.clear();

			AssetRegistry::Get().ForEachPathInDirectoryRecursively(m_SelectedDirectory, [this](const std::string& path)
			{
				auto spec = AssetRegistry::Get().GetPathSpec(path);
				if (!spec->IsAsset()) return;

				if (m_Filter.PassFilter(spec->PathName.c_str()))
				{
					bool bShouldDrawPath = true;
					if (m_bIsAnyTypeFilterActive)
					{
						auto typeId = spec->GetAssetTypeId();
						auto it = std::find_if(m_AssetTypeFilters.begin(), m_AssetTypeFilters.end(), [typeId](const auto& filterSpec)
						{
							return filterSpec.TypeId == typeId;
						});
						bShouldDrawPath = it->bIsFilterActive;
					}
					if (bShouldDrawPath)
					{
						m_FilteredPaths.emplace_back(path);
					}
				}
			});

			m_bShouldUpdateFilterCache = false;
		}

		if (m_ViewType == ContentBrowserViewType::Tiles)
		{
			static constexpr float padding = 16.0f;
			static const float thumbnailWidth = Utils::GetTileThumbnailWidth();
			const float cellSize = thumbnailWidth + padding;
			const float panelWidth = ImGui::GetContentRegionAvail().x;
			int32_t columnCount = static_cast<int32_t>(panelWidth / cellSize);
			if (columnCount < 1)
			{
				columnCount = 1;
			}
			if (ImGui::BeginTable("", columnCount))
			{
				for (auto it = m_FilteredPaths.begin(); it != m_FilteredPaths.end(); ++it)
				{
					ImGui::TableNextColumn();

					DrawTilePath(*it);
				}

				ImGui::EndTable();
			}
		}
		else
		{
			ImGuiListClipper clipper;
			clipper.Begin(static_cast<int32_t>(m_FilteredPaths.size()), Utils::GetSelectableThumbnailWidth());
			auto it = m_FilteredPaths.begin();
			while (clipper.Step())
			{
				for (auto index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
				{
					DrawSelectablePath(*(it + index));
				}
			}
		}

		//END_BENCHMARK()

		if (m_FilteredPaths.empty())
		{
			m_Filter.DrawEmptyText();
		}
	}

	void ContentBrowserPanel::DrawWindowContextMenu()
	{
		// Right-click on blank space
		if (ImGui::BeginPopupContextWindowWithPadding(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			// Actual directory or asset is created after renaming

			const float thumbnailWidth = Utils::GetSelectableThumbnailWidth();

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
				std::string newPath = GetAvailableNewPathName("NewFolder", false);
				RequestPathCreation(newPath, {});
			}

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
					ImGui::Text(factory->GetAssetTypeName());

					if (bIsAssetCreationSelected)
					{
						char baseName[MAX_PATH_SIZE] = "New";
						std::string formattedName = Utils::GetFormatedAssetTypeName(factory->GetAssetTypeName());
						strcat_s(baseName, formattedName.c_str());
						std::string newPath = GetAvailableNewPathName(baseName, true);
						RequestPathCreation(newPath, typeId);
					}
				}
				ImGui::PopID();
			});

			ImGui::EndPopup();
		}
	}

	void ContentBrowserPanel::ClearAllFilters()
	{
		m_Filter.Clear();
		for (auto& typeFilter : m_AssetTypeFilters)
		{
			typeFilter.bIsFilterActive = false;
		}
		m_bIsAnyTypeFilterActive = false;
		m_bIsTypeFilterChanged = true;
	}

	std::string ContentBrowserPanel::GetAvailableNewPathName(const char* baseName, bool bIsAsset)
	{
		int32_t i = 0;
		char suffix[10];
		char newName[MAX_PATH_SIZE];
		strcpy_s(newName, baseName);
		if (bIsAsset)
		{
			strcat_s(newName, AssetRegistry::GetEngineAssetExtension());
		}
		std::string newPath = PathUtils::AppendPath(m_SelectedDirectory, newName);
		while (AssetRegistry::Get().ContainsPathInDirectory(m_SelectedDirectory, newPath))
		{
			_itoa_s(++i, suffix, 10);
			strcpy_s(newName, baseName);
			strcat_s(newName, suffix);
			if (bIsAsset)
			{
				strcat_s(newName, AssetRegistry::GetEngineAssetExtension());
			}
			newPath = PathUtils::AppendPath(m_SelectedDirectory, newName);
		}
		return newPath;
	}

	void ContentBrowserPanel::DrawSelectablePath(const std::string& path)
	{
		// Push path as id
		ImGui::PushID(path.c_str());
		{
			auto spec = AssetRegistry::Get().GetPathSpec(path);
			bool bIsAsset = spec->IsAsset();
			const bool bPathNeedsRenaming = m_PathToRename == path;
			static const float thumbnailWidth = Utils::GetSelectableThumbnailWidth();
			static constexpr float thumbnailRounding = 4.0f;
			const auto pathName = spec->PathName.c_str();

			ImGuiSelectableFlags flags = bPathNeedsRenaming ? ImGuiSelectableFlags_Disabled : 0; // Disable selectable during renaming so that text can be selected
			flags |= ImGuiSelectableFlags_AllowItemOverlap;
			bool bIsSelected = ImGui::Selectable("##PathSelectable", m_SelectedPath == path, flags, { 0.0f, thumbnailWidth });

			// Draw path tooltip on hover
			DrawPathTooltip(spec);

			// Draw path context menu on right click
			DrawPathContextMenu(path);

			// Process asset dragging
			ProcessAssetDragging(spec, thumbnailRounding);

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

			// Draw seletable thumbnail
			ImGui::AssetThumbnail(spec->ThumbnailTexture->GetTextureID(),
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
					ImGui::TextColored({ 0.6f, 0.6f, 0.6f, 1.0f }, bIsAsset ? AssetManager::Get().GetAssetFactoryByAssetType(spec->GetAssetTypeId())->GetAssetTypeName() : "Folder");
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
					SubmitPathRenaming(renameBuffer, MAX_PATH_SIZE, spec, bHasKeyboardFocused);
				}
			}
			ImGui::EndGroup();

			if (bIsSelected)
			{
				m_SelectedPath = path;
			}
		}
		ImGui::PopID();
	}

	void ContentBrowserPanel::DrawTilePath(const std::string& path)
	{
		// Push path as id
		ImGui::PushID(path.c_str());
		{
			auto spec = AssetRegistry::Get().GetPathSpec(path);
			bool bIsAsset = spec->IsAsset();
			const bool bPathNeedsRenaming = m_PathToRename == path;
			static const float thumbnailWidth = Utils::GetTileThumbnailWidth();
			static constexpr float thumbnailRounding = 4.0f;
			const auto pathName = spec->PathName.c_str();

			ImGui::TileImageButton(spec->ThumbnailTexture->GetTextureID(), bPathNeedsRenaming, // Disable button during renaming so that text can be selected
				{ thumbnailWidth, thumbnailWidth }, thumbnailRounding, m_SelectedPath == path,
				{ 0.0f, 1.0f }, { 1.0f, 0.0f });

			// Draw path tooltip on hover
			DrawPathTooltip(spec);

			// Draw path context menu on right click
			DrawPathContextMenu(path);

			// Process asset dragging
			ProcessAssetDragging(spec, thumbnailRounding);

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
			}

			const float textSize = ImGui::CalcTextSize(pathName).x;
			if (!bPathNeedsRenaming)
			{
				// Display directory/asset name
				float indent = (thumbnailWidth - textSize) * 0.5f + ImGui::GetFramePadding().x;
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
				float indent = (thumbnailWidth - textSize) * 0.5f;
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
				SubmitPathRenaming(renameBuffer, MAX_PATH_SIZE, spec, bHasKeyboardFocused);
			}
		}
		ImGui::PopID();
	}

	// TODO: Draw path tooltip
	void ContentBrowserPanel::DrawPathTooltip(const Ref<PathSpec>& spec)
	{
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltipWithPadding();
			ImGui::Text(spec->PathName.c_str());
			ImGui::Separator();
			ImGui::Text("Type: %s", spec->IsAsset() ? AssetManager::Get().GetAssetFactoryByAssetType(spec->GetAssetTypeId())->GetAssetTypeName() : "Folder");
			ImGui::Text("Path: %s", spec->Path.c_str());
			if (spec->IsImportableAsset())
			{
				ImGui::Text("Resource Path: %s", spec->GetResourcePath().c_str());
			}
			ImGui::EndTooltipWithPadding();
		}
	}

	void ContentBrowserPanel::DrawPathContextMenu(const std::string& path)
	{
		if (ImGui::BeginPopupContextItemWithPadding(nullptr))
		{
			m_SelectedPath = path;

			auto spec = AssetRegistry::Get().GetPathSpec(path);
			bool bIsAsset = spec->IsAsset();

			if (ImGui::MenuItem("Rename", "F2"))
			{
				RequestPathRenaming(path);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Rename this file");
			}

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

			if (ImGui::MenuItem("Show In Explorer"))
			{
				PlatformUtils::ShowInExplorer(path);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltipWithPadding("Show in external file explorer");
			}

			ImGui::EndPopup();
		}
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
				Utils::GetTileThumbnailWidth(), thumbnailRounding,
				true, Texture2D::s_DefaultBackgroundTexture->GetTextureID());

			ImGui::EndDragDropSource();
		}
		ImGui::PopStyleColor();
	}

	void ContentBrowserPanel::SubmitPathRenaming(char* renameBuffer, int32_t maxPathSize, const Ref<PathSpec>& spec, bool& bHasKeyboardFocused)
	{
		// ImGui::IsItemDeactivated() will not get called for right-click, so we add these specific check
		if (ImGui::IsItemDeactivated() || (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !ImGui::IsItemHovered()))
		{
			if (spec->IsAsset())
			{
				// Add engine file extension automatically
				strcat_s(renameBuffer, maxPathSize, AssetRegistry::GetEngineAssetExtension());
			}
			const std::string& path = spec->Path;
			std::string parentPath = PathUtils::GetParentPath(path);
			std::string newPath = PathUtils::AppendPath(parentPath, renameBuffer);
			if (newPath != path && AssetRegistry::Get().ContainsPathInDirectory(parentPath, newPath))
			{
				ZE_CORE_WARN("Failed to rename \"{0}\" to \"{1}\"! Path already exist.", path, newPath);
				newPath = path;
			}
			ProcessPathRenaming(path, newPath, spec->GetAssetTypeId());
			bHasKeyboardFocused = false;
			m_bShouldUpdateFilterCache = true;
		}
	}

	void ContentBrowserPanel::HandleRightColumnDirectoryOpen(const std::string& directory)
	{
		ImGuiStorage* storage = ImGui::FindWindowByID(m_LeftColumnWindowId)->DC.StateStorage;
		// Manually toggle upper-level tree node open iteratively
		auto currentDirectory = directory;
		auto& ar = AssetRegistry::Get();
		while (currentDirectory != AssetRegistry::GetAssetRootDirectory())
		{
			storage->SetInt(ar.GetPathSpec<DirectorySpec>(currentDirectory)->TreeNodeId, true);
			currentDirectory = PathUtils::GetParentPath(currentDirectory);
		}
		storage->SetInt(ar.GetPathSpec<DirectorySpec>(currentDirectory)->TreeNodeId, true);
	}

	void ContentBrowserPanel::HandleRightColumnAssetOpen(const std::string& path)
	{
		AssetManager::Get().OpenAsset(path);
	}

	void ContentBrowserPanel::RequestPathCreation(const std::string& path, AssetTypeId typeId)
	{
		m_PathToRename = path;
		m_PathToCreate = path;
		ClearAllFilters(); // Keep filters active during path creation is meaningless
		AssetRegistry::Get().OnPathCreated(path, typeId);
	}

	void ContentBrowserPanel::RequestPathDeletion(const std::string& path)
	{
		if (!path.empty())
		{
			m_PathToDelete = path;
		}
	}

	void ContentBrowserPanel::RequestPathRenaming(const std::string& path)
	{
		if (!path.empty())
		{
			m_PathToRename = path;
		}
	}

	void ContentBrowserPanel::RequestPathOpen(const std::string& path)
	{
		if (!path.empty())
		{
			auto spec = AssetRegistry::Get().GetPathSpec(path);
			if (spec->IsAsset())
			{
				HandleRightColumnAssetOpen(path);
			}
			else
			{
				m_SelectedDirectory = path;
				HandleRightColumnDirectoryOpen(path);
			}
		}
	}

	void ContentBrowserPanel::ProcessPathDeletion(const std::string& path)
	{
		static ImVec2 buttonSize{ 120.0f, 0.0f };
		ImGui::OpenPopup("Delete?");
		// Deletion request dialog
		if (ImGui::BeginPopupModal("Delete?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("The selected folder/asset will be permanently deleted.\nThis operation cannot be undone!\n\n");

			if (ImGui::Button("OK", buttonSize))
			{
				auto spec = AssetRegistry::Get().GetPathSpec(path);
				if (!spec->IsAsset())
				{
					// Delete a directory
					PathUtils::DeletePath(path);
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
				m_bShouldUpdateFilterCache = true;

				ImGui::CloseCurrentPopup();
			}
			ImGui::SetItemDefaultFocus();

			float availableWidth = ImGui::GetContentRegionAvail().x;
			ImGui::SameLine(availableWidth - buttonSize.x);

			if (ImGui::Button("Cancel", buttonSize))
			{
				m_PathToDelete.clear();

				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void ContentBrowserPanel::ProcessPathRenaming(const std::string& oldPath, const std::string& newPath, AssetTypeId typeId)
	{
		if (m_PathToCreate.empty())
		{
			if (newPath != oldPath)
			{
				// Regular renaming process
				PathUtils::RenamePath(oldPath, newPath);
			}
		}
		else
		{
			// Triggered by path creation
			if (typeId)
			{
				AssetManager::Get().CreateAsset(typeId, newPath);
			}
			else
			{
				PathUtils::CreateDirectory(newPath);
			}
			m_PathToCreate.clear();
		}
		m_SelectedPath = newPath;
		if (newPath != oldPath)
		{
			AssetRegistry::Get().OnPathRenamed(oldPath, newPath, typeId);
		}
		m_PathToRename.clear();
	}

}