#include "Panels/ContentBrowserPanel.h"

#include <IconsFontAwesome5.h>

#include "Engine/Core/Assert.h"
#include "Engine/Debug/BenchmarkTimer.h"
#include "Core/AssetManager.h"
#include "Engine/Utils/EngineUtils.h"
#include "Utils/EditorUtils.h"

namespace ZeoEngine {

	void ContentBrowserPanel::OnAttach()
	{
		PreprocessDirectoryHierarchy();
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

	void ContentBrowserPanel::PreprocessDirectoryHierarchy()
	{
		std::filesystem::directory_iterator list(m_AssetRootDirectory);
		for (auto& it : list)
		{
			if (!it.is_directory()) continue;

			const auto& directory = it.path();
			m_DirectoryHierarchy[directory.string()].emplace_back(directory);
			PreprocessDirectoryHierarchyRecursively(directory, directory);
		}
	}

	void ContentBrowserPanel::PreprocessDirectoryHierarchyRecursively(const std::filesystem::path& baseDirectory, const std::filesystem::path& topBaseDirectory)
	{
		std::filesystem::directory_iterator list(baseDirectory);
		for (auto& it : list)
		{
			if (!it.is_directory()) continue;

			const auto& directory = it.path();
			m_DirectoryHierarchy[topBaseDirectory.string()].emplace_back(directory);
			PreprocessDirectoryHierarchyRecursively(directory, topBaseDirectory);
		}
	}

	void ContentBrowserPanel::DrawTopBar()
	{
		// Filters menu
		if (ImGui::BeginPopupWithPadding("Filters"))
		{
			// TODO: AssetType Filter
			ImGui::EndPopup();
		}
		if (ImGui::Button(ICON_FA_FILTER  "Filters"))
		{
			ImGui::OpenPopup("Filters");
		}

		ImGui::SameLine();

		m_Filter.Draw("##ContentBrowserFilter", "Search assets", -1.0f);
	}

	void ContentBrowserPanel::DrawLeftColumn()
	{
		if (ImGui::BeginChild("ContentBrowserLeftColumn", ImVec2(m_LeftColumnWidth, 0.0f)))
		{
			DrawDirectoryHierarchy();
		}

		ImGui::EndChild();
	}

	void ContentBrowserPanel::DrawDirectoryHierarchy()
	{
		m_LeftColumnWindowId = ImGui::GetCurrentWindow()->ID;

		ImGuiTreeNodeFlags flags = (m_SelectedDirectory == m_AssetRootDirectory ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
		static const char* rootDirectoryName = ICON_FA_FOLDER " Assets";
		bool bIsRootTreeExpanded = ImGui::TreeNodeEx(rootDirectoryName, flags);
		m_DirectorySpecs[m_AssetRootDirectory.string()].TreeNodeId = ImGui::GetCurrentWindow()->GetID(rootDirectoryName);
		if (ImGui::IsItemClicked())
		{
			m_SelectedDirectory = m_AssetRootDirectory;
		}
		if (bIsRootTreeExpanded)
		{
			//BenchmarkTimer bt;
			DrawDirectoryHierarchyRecursively(m_AssetRootDirectory);
			//std::cout << bt.ElapsedMillis() << std::endl;

			ImGui::TreePop();
		}
	}

	void ContentBrowserPanel::DrawDirectoryHierarchyRecursively(const std::filesystem::path& baseDirectory)
	{
		std::filesystem::directory_iterator list(baseDirectory);
		for (auto& it : list)
		{
			if (!it.is_directory()) continue;

			char directoryName[128] = ICON_FA_FOLDER " ";
			const auto& directory = it.path();
			strcat_s(directoryName, directory.filename().string().c_str());
			ImGuiTreeNodeFlags flags = (m_SelectedDirectory == directory ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
			bool bHasAnySubDirectory = false;
			const auto direcotyStr = directory.string();
			if (m_DirectorySpecs[direcotyStr].bHasSubDirectory)
			{
				bHasAnySubDirectory = *m_DirectorySpecs[direcotyStr].bHasSubDirectory;
			}
			else
			{
				std::filesystem::directory_iterator subList(directory);
				for (auto& subIt : subList)
				{
					if (subIt.is_directory())
					{
						bHasAnySubDirectory = true;
						break;
					}
				}
				m_DirectorySpecs[direcotyStr].bHasSubDirectory = bHasAnySubDirectory;
			}
			if (!bHasAnySubDirectory)
			{
				flags |= ImGuiTreeNodeFlags_Leaf;
			}
			bool bIsTreeExpanded = ImGui::TreeNodeEx(directoryName, flags);
			m_DirectorySpecs[direcotyStr].TreeNodeId = ImGui::GetCurrentWindow()->GetID(directoryName);
			if (ImGui::IsItemClicked())
			{
				m_SelectedDirectory = directory;
			}
			if (bIsTreeExpanded)
			{
				DrawDirectoryHierarchyRecursively(directory);

				ImGui::TreePop();
			}
		}
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
			DrawDirectorySelector();

			ImGui::Separator();

			if (!m_Filter.IsActive())
			{
				DrawPathsInDirectory();
			}
			else
			{
				DrawFilteredAssetsInDirectoryRecursively();
			}
		}

		ImGui::EndChild();
	}

	void ContentBrowserPanel::DrawDirectorySelector()
	{
		for (auto it = m_SelectedDirectory.begin(); it != m_SelectedDirectory.end(); ++it)
		{
			ImGui::SameLine(0.0f, 0.0f);
			if (ImGui::TransparentSmallButton(it->string().c_str()))
			{
				std::filesystem::path jumpToDirectory;
				for (auto it2 = m_SelectedDirectory.begin(); it2 != m_SelectedDirectory.end(); ++it2)
				{
					jumpToDirectory /= *it2;
					if (it2 == it) break;
				}
				m_SelectedDirectory = jumpToDirectory;
			}
			ImGui::SameLine(0.0f, 0.0f);
			ImGui::Text("/");
		}
	}

	void ContentBrowserPanel::DrawPathsInDirectory()
	{
		std::filesystem::directory_iterator list(m_SelectedDirectory);
		bool bIsDirectoryEmpty = true;
		for (auto& it : list)
		{
			const auto& path = it.path();
			switch (it.status().type())
			{
				// Draw directory
				case std::filesystem::file_type::directory:
				{
					bIsDirectoryEmpty = false;
					DrawDirectory(path);

					break;
				}
				// Draw asset
				case std::filesystem::file_type::regular:
				{
					if (path.extension().string() == g_EngineAssetExtension)
					{
						bIsDirectoryEmpty = false;
						DrawAsset(path);
					}
					break;
				}
			}
		}

		if (bIsDirectoryEmpty)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			ImGui::TextCentered("This folder is empty");
			ImGui::PopStyleColor();
		}
	}

	void ContentBrowserPanel::DrawFilteredAssetsInDirectoryRecursively()
	{
		bool bIsFilteredEmpty = true;
		std::filesystem::recursive_directory_iterator list(m_SelectedDirectory);
		for (auto& it : list)
		{
			if (!it.is_regular_file()) continue;

			const auto& path = it.path();
			if (path.extension().string() == g_EngineAssetExtension
				&& m_Filter.PassFilter(path.stem().string().c_str()))
			{
				bIsFilteredEmpty = false;
				DrawAsset(path);
			}
		}

		if (bIsFilteredEmpty)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			ImGui::TextCentered("There is nothing to display -_-");
			ImGui::PopStyleColor();
		}
	}

	void ContentBrowserPanel::DrawSelectablePath(const char* name, const std::filesystem::path& path)
	{
		static constexpr float indentWidth = 5.0f;
		ImGui::Indent(indentWidth);
		if (ImGui::Selectable(name, m_SelectedPath == path))
		{
			m_SelectedPath = path;
		}
		ImGui::Unindent(indentWidth);
	}

	void ContentBrowserPanel::DrawDirectory(const std::filesystem::path& path)
	{
		char directoryName[128] = ICON_FA_FOLDER " ";
		strcat_s(directoryName, path.stem().string().c_str());
		DrawSelectablePath(directoryName, path);
		// Double-click to open directory in the right column
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
		{
			m_SelectedDirectory = path;

			HandleRightColumnDirectoryDoubleClicked(path);
		}
	}

	void ContentBrowserPanel::DrawAsset(const std::filesystem::path& path)
	{
		const std::string pathStr = path.string();
		if (m_AssetIcons.find(pathStr) == m_AssetIcons.end())
		{
			AssetType assetType = FileUtils::GetAssetTypeFromFile(path.string());
			m_AssetIcons[pathStr] = EditorUtils::GetAssetIcon(assetType);
		}

		char assetName[128] = "";
		strcat_s(assetName, m_AssetIcons[pathStr]);
		strcat_s(assetName, path.stem().string().c_str());
		DrawSelectablePath(assetName, path);
		// Double-click to open asset editor
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
		{
			HandleRightColumnAssetDoubleClicked(path);
		}
	}

	void ContentBrowserPanel::HandleRightColumnDirectoryDoubleClicked(const std::filesystem::path& directory)
	{
		ImGuiStorage* storage = ImGui::FindWindowByID(m_LeftColumnWindowId)->DC.StateStorage;
		// Manually toggle upper-level tree node open iteratively
		for (const auto& [topBaseDirectory, subDirectories] : m_DirectoryHierarchy)
		{
			for (auto i = 0; i < subDirectories.size(); ++i)
			{
				if (subDirectories[i] == directory)
				{
					for (auto j = i; j >= 0; --j)
					{
						storage->SetInt(m_DirectorySpecs[subDirectories[j].string()].TreeNodeId, true);
					}
					storage->SetInt(m_DirectorySpecs[m_AssetRootDirectory.string()].TreeNodeId, true);
					break;
				}
			}
		}
	}

	void ContentBrowserPanel::HandleRightColumnAssetDoubleClicked(const std::filesystem::path& path)
	{
		AssetManager::Get().OpenAsset(path.string());
	}

}
