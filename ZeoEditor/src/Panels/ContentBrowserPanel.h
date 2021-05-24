#pragma once

#include "Panels/PanelBase.h"

#include <filesystem>
#include <optional>

#include "Engine/Events/KeyEvent.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/ImGui/TextFilter.h"

namespace ZeoEngine {

	struct DirectorySpec
	{
		uint32_t TreeNodeId;
		std::optional<bool> bHasSubDirectory;
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

		void ConstructDirectoryHierarchy();
		void ConstructDirectoryHierarchyRecursively(const std::filesystem::path& baseDirectory);
		/**
		 * Sorting:
		 * Directories first, assets second
		 * In alphabetical order
		 */
		void SortDirectoryHierarchy();

		void DrawTopBar();
		void DrawLeftColumn();
		void DrawColumnSplitter(float contentWidth);
		void DrawRightColumn();

		void DrawDirectoryHierarchy();
		void DrawDirectoryHierarchyRecursively(const std::filesystem::path& baseDirectory);

		void DrawDirectoryNavigator();
		void DrawPathsInDirectory();
		void DrawFilteredAssetsInDirectoryRecursively();
		void DrawWindowContextMenu();
		void DrawSelectablePath(const char* name, const std::filesystem::path& path);
		void DrawDirectory(const std::filesystem::path& path);
		void DrawAsset(const std::filesystem::path& path);

		void HandleRightColumnDirectoryDoubleClicked(const std::filesystem::path& directory);
		void HandleRightColumnAssetDoubleClicked(const std::filesystem::path& path);

		/** Called when a new directory or asset is created within the ContentBrowser. */
		void OnPathCreated(const std::filesystem::path& path);
		/** Called when an existing directory or asset is removed within the ContentBrowser. */
		void OnPathRemoved(const std::filesystem::path& path);
		/** Called when a directory or asset is renamed within the ContentBrowser. */
		void OnPathRenamed(const std::filesystem::path& oldPath, const std::filesystem::path& newPath);

	private:
		const std::filesystem::path m_AssetRootDirectory{ std::filesystem::path{ "assets" } };

		float m_LeftColumnWidth = 200.0f;
		uint32_t m_LeftColumnWindowId;

		/** Selected directory in the left column */
		std::filesystem::path m_SelectedDirectory{ m_AssetRootDirectory };
		/** Selected directory/asset in the right column */
		std::filesystem::path m_SelectedPath;
		/** Directory or asset just created within the ContentBrowser */
		std::filesystem::path m_PathJustCreated;

		/** Map from base path to list of its direct sub-directories and assets in order */
		std::vector<std::pair<std::filesystem::directory_entry, std::vector<std::filesystem::directory_entry>>> m_DirectoryHierarchy;
		/** Map from directory string to directory specification */
		std::unordered_map<std::string, DirectorySpec> m_DirectorySpecs;
		/** Map from asset path string to asset icon */
		std::unordered_map<std::string, const char*> m_AssetIcons;

		TextFilter m_Filter;
	};

}
