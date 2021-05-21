#pragma once

#include "Panels/PanelBase.h"

#include <filesystem>
#include <optional>
#include <array>

#include "Engine/Core/EngineTypes.h"

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
		void PreprocessDirectoryHierarchyRecursively(const std::filesystem::path& baseDirectory, const std::filesystem::path& topBaseDirectory);

		void DrawLeftColumn();
		void DrawColumnSplitter(float contentWidth);
		void DrawRightColumn();
		void DrawDirectoryHierarchy();
		void DrawDirectoryHierarchyRecursively(const std::filesystem::path& baseDirectory);
		void DrawAssetsInDirectory();

		void HandleRightColumnDirectoryDoubleClicked(const std::filesystem::path& directory);
		void HandleRightColumnAssetDoubleClicked(const std::filesystem::path& path);

	private:
		const std::filesystem::path m_AssetRootDirectory{ std::filesystem::path{ "assets" } };

		float m_LeftColumnWidth = 200.0f;
		uint32_t m_LeftColumnWindowId;

		/** Selected folder in the left column */
		std::filesystem::path m_SelectedDirectory{ m_AssetRootDirectory };
		/** Selected folder/asset in the right column */
		std::filesystem::path m_SelectedPath;
		/** Map from directory string to directory specification */
		std::unordered_map<std::string, DirectorySpec> m_DirectorySpecs;
		/** Map from top base directory string to list of itself and its sub-directories */
		std::unordered_map<std::string, std::vector<std::filesystem::path>> m_DirectoryHierarchy;
	};

}
