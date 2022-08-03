#pragma once

#include "Engine/Asset/Asset.h"
#include "Panels/PanelBase.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/ImGui/TextFilter.h"

namespace ZeoEngine {

	struct PathMetadata;
	struct AssetMetadata;

	class AssetBrowserPanelBase : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

		virtual void OnAttach() override;

		const auto& GetSelectedDirectory() const { return m_SelectedDirectory; }
		void SetSelectedDirectory(std::filesystem::path directory) { m_SelectedDirectory = std::move(directory); }
		const auto& GetSelectedPath() const { return m_SelectedPath; }
		void SetSelectedPath(const std::filesystem::path& path) { m_SelectedPath = path; SetSelectedDirectory(path.parent_path()); m_bFocusSelectedPath = true; }

	protected:
		constexpr float GetSelectableThumbnailWidth() const { return 32.0f; }
		constexpr float GetTileThumbnailWidth() const { return 64.0f; }
		constexpr float GetTableOffset() const { return 4.0f; }

		void SetForceUpdateFilterCache(bool bValue) { m_bForceUpdateFilterCache = bValue; }
		virtual bool IsAnyFilterActive() const { return m_Filter.IsActive(); }
		virtual bool ShouldUpdateFilterCache() const { return m_Filter.bIsInputBufferChanged || m_bForceUpdateFilterCache; }
		virtual bool PassFilter(const Ref<PathMetadata>& metadata) const { return true; }
		/** Clear type filter and search filter. */
		virtual void ClearAllFilters();

		std::string GetFormattedAssetTypeName(const char* typeName) const;

		/** Try to find an available path name in current directory by appending suffix to it. */
		std::filesystem::path GetAvailableNewPathName(const char* baseName, bool bIsAsset) const;

		void RequestPathCreation(const std::filesystem::path& path, AssetTypeID typeID, bool bNeedsRenaming);
		void RequestPathCreationForResourceAsset(const std::filesystem::path& srcPath, const std::filesystem::path& destPath);
		void RequestPathDeletion(const std::filesystem::path& path);
		void RequestPathRenaming(const std::filesystem::path& path);
		void RequestPathOpen(const std::filesystem::path& path);

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
		void DrawDirectoryTreeRecursively(const std::filesystem::path& baseDirectory);

		void DrawDirectoryNavigator();
		void DrawPathsInDirectory();
		void DrawFilteredAssetsInDirectoryRecursively();
		void DrawWindowContextMenu();
		virtual void DrawWindowContextMenuImpl(float thumbnailWidth) {}

		virtual bool ShouldDrawPath(const Ref<PathMetadata>& metadata) { return true; }
		void DrawSelectablePath(const Ref<PathMetadata>& metadata);
		void DrawTilePath(const Ref<PathMetadata>& metadata);
		virtual void OnPathSelected(const std::filesystem::path& path) {}

		void DrawPathTooltip(const Ref<PathMetadata>& metadata) const;
		void DrawPathContextMenu(const std::filesystem::path& path);
		virtual void DrawPathContextMenuItem_Save(const std::filesystem::path& path, bool bIsAsset) {}
		virtual void DrawPathContextMenuItem_Asset(const std::filesystem::path& path, const Ref<AssetMetadata>& metadata) {}

		virtual void ProcessAssetDragging(const Ref<PathMetadata>& metadata, float thumbnailRounding) {}
		void SubmitPathRenaming(char* renameBuffer, const Ref<PathMetadata>& metadata, bool& bHasKeyboardFocused);

		void HandleRightColumnDirectoryOpen(const std::filesystem::path& directory);
		virtual void HandleRightColumnAssetOpen(const std::filesystem::path& path) = 0;

		void ProcessPathDeletion(const std::filesystem::path& path);
		void ProcessPathRenaming(const std::filesystem::path& oldPath, const std::filesystem::path& newPath, AssetTypeID typeID);

	private:
		float m_LeftColumnWidth = 200.0f;
		U32 m_LeftColumnWindowId;

		enum class AssetBrowserViewType
		{
			Tiles,
			List,
		};
		AssetBrowserViewType m_ViewType = AssetBrowserViewType::Tiles;

		/** Selected directory in the left column */
		std::filesystem::path m_SelectedDirectory;
		/** Selected directory/asset in the right column */
		std::filesystem::path m_SelectedPath;
		/** Flag used to scroll to the selected path once */
		bool m_bFocusSelectedPath = false;

		/** Directory waiting for opening */
		std::filesystem::path m_DirectoryToOpen;
		/** Directory or asset waiting for renaming */
		std::filesystem::path m_PathToRename;
		/** Directory or asset waiting for creation */
		std::filesystem::path m_PathToCreate;
		/** Directory or asset waiting for deletion */
		std::filesystem::path m_PathToDelete;

		TextFilter m_Filter;

		/** Filter cache */
		std::vector<std::filesystem::path> m_FilteredPaths;
		bool m_bForceUpdateFilterCache = false;
	};

}
