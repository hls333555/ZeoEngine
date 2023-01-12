#pragma once

#include "Engine/Asset/Asset.h"
#include "Panels/PanelBase.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Core/EngineTypes.h"
#include "Widgets/TextFilter.h"
#include "Engine/Utils/FileSystemUtils.h"

namespace ZeoEngine {

	struct PathMetadata;
	struct AssetMetadata;

	class AssetBrowserPanelBase : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

		virtual void OnAttach() override;

		const auto& GetSelectedDirectory() const { return m_SelectedDirectory; }
		void SetSelectedDirectory(std::string directory) { m_SelectedDirectory = std::move(directory); }
		const auto& GetSelectedPath() const { return m_SelectedPath; }
		void SetSelectedPath(const std::string& path) { m_SelectedPath = path; SetSelectedDirectory(FileSystemUtils::GetParentPath(path)); m_bFocusSelectedPath = true; }

	protected:
		constexpr float GetSelectableThumbnailWidth() const { return 32.0f; }
		constexpr float GetTileThumbnailWidth() const { return 64.0f; }
		constexpr float GetTableOffset() const { return 4.0f; }

		void SetForceUpdateFilterCache(bool bValue) { m_bForceUpdateFilterCache = bValue; }
		virtual bool IsAnyFilterActive() const { return m_Filter.IsActive(); }
		virtual bool ShouldUpdateFilterCache() const { return m_Filter.bIsInputBufferChanged || m_bForceUpdateFilterCache; }
		virtual bool PassFilter(const PathMetadata* metadata) const { return true; }
		/** Clear type filter and search filter. */
		virtual void ClearAllFilters();

		std::string GetFormattedAssetTypeName(const char* typeName) const;

		/** Try to find an available path name in current directory by appending suffix to it. */
		std::string GetAvailableNewPathName(const char* baseName, bool bIsAsset) const;

		void RequestPathCreation(const std::string& path, AssetTypeID typeID, bool bNeedsRenaming);
		void RequestPathCreationForResourceAsset(const std::string& srcPath, const std::string& destPath);
		void RequestPathDeletion(const std::string& path);
		void RequestPathRenaming(const std::string& path);
		void RequestPathOpen(const std::string& path);

		virtual void ProcessEvent(Event& e) override;
	private:
		virtual void ProcessRender() override;

		bool OnKeyPressed(KeyPressedEvent& e);
		
	protected:
		virtual void DrawTopBar();
	private:
		void DrawLeftColumn();
		void DrawColumnSplitter(float contentWidth);
		void DrawRightColumn();
		virtual bool WillDrawBottom() const { return false; }
		virtual void DrawBottom() {}

		void DrawDirectoryTree();
		void DrawDirectoryTreeRecursively(const std::string& baseDirectory);

		void DrawDirectoryNavigator();
		void DrawPathsInDirectory();
		void DrawFilteredAssetsInDirectoryRecursively();
		void DrawWindowContextMenu();
		virtual void DrawWindowContextMenuImpl(float thumbnailWidth) {}

		virtual bool ShouldDrawPath(const PathMetadata* metadata) { return true; }
		void DrawSelectablePath(const PathMetadata* metadata);
		void DrawTilePath(const PathMetadata* metadata);
		virtual void OnPathSelected(const std::string& path) {}

		void DrawPathTooltip(const PathMetadata* metadata) const;
		void DrawPathContextMenu(const std::string& path);
		virtual void DrawPathContextMenuItem_Save(const std::string& path, bool bIsAsset) {}
		virtual void DrawPathContextMenuItem_Asset(const std::string& path, const AssetMetadata* metadata) {}

		virtual void ProcessAssetDragging(const PathMetadata* metadata, float thumbnailRounding) {}
		void SubmitPathRenaming(char* renameBuffer, const PathMetadata* metadata, bool& bHasKeyboardFocused);

		void HandleRightColumnDirectoryOpen(const std::string& directory);
		virtual void HandleRightColumnAssetOpen(const std::string& path) = 0;

		void ProcessPathDeletion(const std::string& path);
		void ProcessPathRenaming(const std::string& oldPath, const std::string& newPath, AssetTypeID typeID);

	private:
		bool m_bFirstFrame = true;
		float m_LeftColumnWidth = 200.0f;
		U32 m_LeftColumnWindowID;

		enum class AssetBrowserViewType
		{
			Tiles,
			List,
		};
		AssetBrowserViewType m_ViewType = AssetBrowserViewType::Tiles;

		/** Selected directory in the left column */
		std::string m_SelectedDirectory;
		/** Selected directory/asset in the right column */
		std::string m_SelectedPath;
		/** Flag used to scroll to the selected path once */
		bool m_bFocusSelectedPath = false;

		/** Directory waiting for opening */
		std::string m_DirectoryToOpen;
		/** Directory or asset waiting for renaming */
		std::string m_PathToRename;
		/** Directory or asset waiting for creation */
		std::string m_PathToCreate;
		/** Directory or asset waiting for deletion */
		std::string m_PathToDelete;

		TextFilter m_Filter;

		/** Filter cache */
		std::vector<std::string> m_FilteredPaths;
		bool m_bForceUpdateFilterCache = false;
	};

}
