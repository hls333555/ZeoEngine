#pragma once

#include "Panels/PanelBase.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/ImGui/TextFilter.h"

namespace ZeoEngine {

	struct PathSpec;

	constexpr uint32_t GetMaxPathSize() { return 128; }

	class AssetBrowserPanelBase : public PanelBase
	{
	public:
		using PanelBase::PanelBase;

		virtual void OnAttach() override;

		const std::string& GetSelectedDirectory() const { return m_SelectedDirectory; }
		const std::string& GetSelectedPath() const { return m_SelectedPath; }

	protected:
		float GetSelectableThumbnailWidth() const { return 32.0f; }
		float GetTileThumbnailWidth() const { return 64.0f; }

		void SetForceUpdateFilterCache(bool bValue) { m_bForceUpdateFilterCache = bValue; }
		virtual bool IsAnyFilterActive() const { return m_Filter.IsActive(); }
		virtual bool ShouldUpdateFilterCache() const { return m_Filter.bIsInputBufferChanged || m_bForceUpdateFilterCache; }
		virtual bool PassFilter(const Ref<PathSpec>& spec) const { return true; }
		/** Clear type filter and search filter. */
		virtual void ClearAllFilters();

		std::string GetFormatedAssetTypeName(const char* typeName);

		/** Try to find an available path name in current directory by appending suffix to it. */
		std::string GetAvailableNewPathName(const char* baseName, bool bIsAsset);

		void RequestPathCreation(const std::string& path, AssetTypeId typeId, bool bNeedsRenaming);
		void RequestPathDeletion(const std::string& path);
		void RequestPathRenaming(const std::string& path);
		void RequestPathOpen(const std::string& path);

	private:
		virtual void ProcessRender() override;
		virtual void ProcessEvent(Event& e);

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

		virtual bool ShouldDrawPath(const Ref<PathSpec>& spec) { return true; }
		void DrawSelectablePath(const std::string& path);
		void DrawTilePath(const std::string& path);
		virtual void OnPathSelected(const std::string& path) {}

		void DrawPathTooltip(const Ref<PathSpec>& spec);
		void DrawPathContextMenu(const std::string& path);
		virtual void DrawPathContextMenuItem_Save(const std::string& path, bool bIsAsset) {}
		virtual void DrawPathContextMenuItem_Asset(const std::string& path, const Ref<PathSpec>& spec) {}

		virtual void ProcessAssetDragging(const Ref<PathSpec>& spec, float thumbnailRounding) {}
		void SubmitPathRenaming(char* renameBuffer, const Ref<PathSpec>& spec, bool& bHasKeyboardFocused);

		void HandleRightColumnDirectoryOpen(const std::string& directory);
		virtual void HandleRightColumnAssetOpen(const std::string& path) = 0;

		void ProcessPathDeletion(const std::string& path);
		void ProcessPathRenaming(const std::string& oldPath, const std::string& newPath, AssetTypeId typeId);

	private:
		float m_LeftColumnWidth = 200.0f;
		uint32_t m_LeftColumnWindowId;

		enum class ContentBrowserViewType
		{
			Tiles,
			List,
		};
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

		TextFilter m_Filter;

		/** Filter cache */
		std::vector<std::string> m_FilteredPaths;
		bool m_bForceUpdateFilterCache = false;
	};

}
