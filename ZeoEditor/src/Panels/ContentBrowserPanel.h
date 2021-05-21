#pragma once

#include "Panels/PanelBase.h"

#include <filesystem>
#include <optional>

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

		void PreprocessDirectoryHierarchy();
		void PreprocessDirectoryHierarchyRecursively(const std::filesystem::path& baseDirectory);
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
		void DrawSelectablePath(const char* name, const std::filesystem::path& path);
		void DrawDirectory(const std::filesystem::path& path);
		void DrawAsset(const std::filesystem::path& path);

		void HandleRightColumnDirectoryDoubleClicked(const std::filesystem::path& directory);
		void HandleRightColumnAssetDoubleClicked(const std::filesystem::path& path);

		/** Called when a new directory or asset is created in ContentBrowser. */
		void OnPathCreated(const std::filesystem::path& path);
		/** Called when an existing directory or asset is removed from ContentBrowser. */
		void OnPathRemoved(const std::filesystem::path& path);

	private:
		const std::filesystem::path m_AssetRootDirectory{ std::filesystem::path{ "assets" } };

		float m_LeftColumnWidth = 200.0f;
		uint32_t m_LeftColumnWindowId;

		/** Selected folder in the left column */
		std::filesystem::path m_SelectedDirectory{ m_AssetRootDirectory };
		/** Selected folder/asset in the right column */
		std::filesystem::path m_SelectedPath;
		/** Map from base path to list of its direct sub-directories and assets in order */
		std::vector<std::pair<std::filesystem::directory_entry, std::vector<std::filesystem::directory_entry>>> m_DirectoryHierarchy;
		/** Map from directory string to directory specification */
		std::unordered_map<std::string, DirectorySpec> m_DirectorySpecs;
		/** Map from asset path string to asset icon */
		std::unordered_map<std::string, const char*> m_AssetIcons;

		TextFilter m_Filter;
	};

}
