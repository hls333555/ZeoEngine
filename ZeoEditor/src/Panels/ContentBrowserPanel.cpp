#include "Panels/ContentBrowserPanel.h"

#include <IconsFontAwesome5.h>

#include "Engine/Core/Assert.h"
#include "Engine/Debug/BenchmarkTimer.h"

namespace ZeoEngine {

	void ContentBrowserPanel::OnAttach()
	{
		PreprocessDirectoryHierarchy();
	}

	void ContentBrowserPanel::ProcessRender()
	{
		float contentWidth = ImGui::GetContentRegionAvail().x;

		DrawLeftColumn();

		ImGui::SameLine();

		DrawColumnSplitter(contentWidth);

		ImGui::SameLine();

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

	void ContentBrowserPanel::DrawLeftColumn()
	{
		DrawDirectoryHierarchy();
	}

	void ContentBrowserPanel::DrawDirectoryHierarchy()
	{
		ImVec2 size(m_LeftColumnWidth, 0.0f);
		if (ImGui::BeginChild("ContentBrowserLeftColumn", size))
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

		ImGui::EndChild();
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
		static const float columnPadding = 15.0f;
		m_LeftColumnWidth = glm::clamp(leftColumnSize.x, columnPadding, contentWidth - columnPadding);
	}

	void ContentBrowserPanel::DrawRightColumn()
	{
		DrawAssetsInDirectory();
	}

	void ContentBrowserPanel::DrawAssetsInDirectory()
	{
		if (ImGui::BeginChild("ContentBrowserRightColumn"))
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

						char directoryName[128] = ICON_FA_FOLDER " ";
						strcat_s(directoryName, path.filename().string().c_str());
						if (ImGui::Selectable(directoryName, m_SelectedPath == path))
						{
							m_SelectedPath = path;
						}
						// Double-click to open directory in the right column
						if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
						{
							m_SelectedDirectory = path;

							HandleRightColumnDirectoryDoubleClicked(path);
						}
						break;
					}
					// Draw asset
					case std::filesystem::file_type::regular:
					{
						if (std::find(m_SupportedAssetExtensions.cbegin(), m_SupportedAssetExtensions.cend(), path.extension().string()) != m_SupportedAssetExtensions.cend())
						{
							bIsDirectoryEmpty = false;

							char assetName[128] = ICON_FA_FILE " ";
							strcat_s(assetName, path.filename().string().c_str());
							if (ImGui::Selectable(assetName, m_SelectedPath == path))
							{
								m_SelectedPath = path;
							}
							if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
							{
								HandleRightColumnAssetDoubleClicked(path);
							}
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

		ImGui::EndChild();
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
		
	}

}
