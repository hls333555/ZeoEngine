#pragma once

#include "Panels/PanelBase.h"

#include "Engine/Core/Core.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/ImGui/TextFilter.h"

namespace ZeoEngine {

	struct PathSpec;

	enum class ContentBrowserViewType
	{
		Tiles,
		List,
	};

	class ContentBrowserPanel : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

		virtual void OnAttach() override;

	private:
		virtual void ProcessRender() override;
		virtual void ProcessEvent(Event& e);

		bool OnKeyPressed(KeyPressedEvent& e);

		void InitAssetTypeFilters();

		void DrawTopBar();
		void DrawLeftColumn();
		void DrawColumnSplitter(float contentWidth);
		void DrawRightColumn();

		void DrawDirectoryTree();
		void DrawDirectoryTreeRecursively(const std::string& baseDirectory);

		void DrawDirectoryNavigator();
		void DrawPathsInDirectory();
		void DrawFilteredAssetsInDirectoryRecursively();
		void DrawWindowContextMenu();

		/** Clear type filter and search filter. */
		void ClearAllFilters();

		/** Try to find an available path name in current directory by appending suffix to it. */
		std::string GetAvailableNewPathName(const char* baseName, bool bIsAsset);

		void DrawSelectablePath(const std::string& path);
		void DrawTilePath(const std::string& path);

		void DrawPathTooltip(const Ref<PathSpec>& spec);
		void DrawPathContextMenu(const std::string& path);
		void ProcessAssetDragging(const Ref<PathSpec>& spec, float thumbnailRounding);
		void SubmitPathRenaming(char* renameBuffer, int32_t maxPathSize, const Ref<PathSpec>& spec, bool& bHasKeyboardFocused);

		void HandleRightColumnDirectoryOpen(const std::string& directory);
		void HandleRightColumnAssetOpen(const std::string& path);

		void RequestPathCreation(const std::string& path, AssetTypeId typeId);
		void RequestPathDeletion(const std::string& path);
		void RequestPathRenaming(const std::string& path);
		void RequestPathOpen(const std::string& path);

		void ProcessPathDeletion(const std::string& path);
		void ProcessPathRenaming(const std::string& oldPath, const std::string& newPath, AssetTypeId typeId);

	private:
		float m_LeftColumnWidth = 200.0f;
		uint32_t m_LeftColumnWindowId;

		ContentBrowserViewType m_ViewType = ContentBrowserViewType::Tiles;

		/** Selected directory in the left column */
		std::string m_SelectedDirectory;
		/** Selected directory/asset in the right column */
		std::string m_SelectedPath;

		/** Directory waiting for opening */
		std::string m_DirectoryToOpen;
		/** Directory or asset waiting for renaming */
		std::string m_PathToRename;
		/** Directory or asset waiting for creation */
		std::string m_PathToCreate;
		/** Directory or asset waiting for deletion */
		std::string m_PathToDelete;

		struct AssetTypeFilterSpec
		{
			AssetTypeId TypeId;
			const char* TypeName;
			bool bIsFilterActive = false;
		};
		std::vector<AssetTypeFilterSpec> m_AssetTypeFilters;
		bool m_bIsAnyTypeFilterActive = false;
		bool m_bIsTypeFilterChanged = false;

		TextFilter m_Filter;

		/** Filter cache */
		std::vector<std::string> m_FilteredPaths;
		bool m_bShouldUpdateFilterCache = false;
	};

}
