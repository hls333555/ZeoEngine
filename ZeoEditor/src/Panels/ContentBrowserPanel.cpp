#include "Panels/ContentBrowserPanel.h"

#include <IconsFontAwesome5.h>
#include <magic_enum.hpp>

#include "Engine/Core/Assert.h"
#include "Engine/Debug/BenchmarkTimer.h"
#include "Core/AssetManager.h"
#include "Engine/Utils/EngineUtils.h"
#include "Utils/EditorUtils.h"
#include "Core/AssetManager.h"
#include "Core/AssetFactory.h"
#include "Core/AssetActions.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/GameFramework/ParticleSystem.h"

#define PATH_BUFFER_SIZE 128

namespace ZeoEngine {

	namespace Utils {

		static bool ShouldAddPath(const std::filesystem::directory_entry& entry)
		{
			switch (entry.status().type())
			{
				case std::filesystem::file_type::directory:
					return true;
				case std::filesystem::file_type::regular:
					if (entry.path().extension().string() == g_EngineAssetExtension)
					{
						return true;
					}
			}
			return false;
		}

		static std::string FileNameToUpperCase(const std::filesystem::path& fileName)
		{
			auto str = fileName.string();
			std::transform(str.begin(), str.end(), str.begin(), ::toupper);
			return str;
		}

	}

	void ContentBrowserPanel::OnAttach()
	{
		BenchmarkTimer timer;
		ConstructDirectoryHierarchy();
		ZE_CORE_WARN("Directory hierarchy construction took {0} ms", timer.ElapsedMillis());

		AssetManager::Get().RegisterAssetFactory(AssetType<Scene>::Id(), CreateRef<SceneAssetFactory>());
		AssetManager::Get().RegisterAssetFactory(AssetType<ParticleTemplate>::Id(), CreateRef<ParticleTemplateAssetFactory>());
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
			// Delete selected directory or asset
			case Key::Delete:
				if (!m_SelectedPath.empty())
				{
					// TODO: Pop up a deletion dialog
					std::filesystem::remove_all(m_SelectedPath);
					OnPathRemoved(m_SelectedPath);
					m_SelectedPath.clear();
				}
				break;
			// TODO: FIXME
			// Rename selected directory or asset
			case Key::F2:
				if (!m_SelectedPath.empty())
				{
					m_PathJustCreated = m_SelectedPath;
				}
				break;
			// Open selected directory or asset
			case Key::Enter:
				if (!m_SelectedPath.empty())
				{
					switch (std::filesystem::directory_entry{ m_SelectedPath }.status().type())
					{
						case std::filesystem::file_type::directory:
							m_SelectedDirectory = m_SelectedPath;
							HandleRightColumnDirectoryDoubleClicked(m_SelectedPath);
							break;
						case std::filesystem::file_type::regular:
							HandleRightColumnAssetDoubleClicked(m_SelectedPath);
							break;
					}
				}
				break;
			default:
				break;
		}

		return false;
	}

	void ContentBrowserPanel::ConstructDirectoryHierarchy()
	{
		m_DirectoryHierarchy.emplace_back(std::make_pair(m_AssetRootDirectory, std::vector<std::filesystem::directory_entry>{}));

		std::filesystem::directory_iterator list(m_AssetRootDirectory);
		for (auto& it : list)
		{
			if (Utils::ShouldAddPath(it))
			{
				m_DirectoryHierarchy[0].second.emplace_back(it);
				m_DirectoryHierarchy.emplace_back(std::make_pair(it, std::vector<std::filesystem::directory_entry>{}));
			}

			if (it.is_directory())
			{
				ConstructDirectoryHierarchyRecursively(it.path());
			}
		}

		SortDirectoryHierarchy();
	}

	void ContentBrowserPanel::ConstructDirectoryHierarchyRecursively(const std::filesystem::path& baseDirectory)
	{
		std::filesystem::directory_iterator list(baseDirectory);
		for (auto& it : list)
		{
			if (Utils::ShouldAddPath(it))
			{
				auto result = std::find_if(m_DirectoryHierarchy.begin(), m_DirectoryHierarchy.end(), [&baseDirectory](const auto& pair)
				{
					return pair.first == baseDirectory;
				});
				if (result != m_DirectoryHierarchy.cend())
				{
					result->second.emplace_back(it);
				}
				m_DirectoryHierarchy.emplace_back(std::make_pair(it, std::vector<std::filesystem::directory_entry>{}));
			}

			if (it.is_directory())
			{
				ConstructDirectoryHierarchyRecursively(it.path());
			}
		}
	}

	void ContentBrowserPanel::SortDirectoryHierarchy()
	{
		for (auto& [baseDirectory, subDirectories] : m_DirectoryHierarchy)
		{
			std::sort(subDirectories.begin(), subDirectories.end(), [](const auto& lhs, const auto& rhs)
			{
				return lhs.is_directory() && !rhs.is_directory() ||
					(((lhs.is_directory() && rhs.is_directory()) || (!lhs.is_directory() && !rhs.is_directory())) &&
					// Default comparison behaviour for directories: "NewFolder" is ahead of "cache"
					// So we have to convert file name to upper or lower case before comparing
					// Comparison behaviour after conversion: "cache" is ahead of "NewFolder"
					Utils::FileNameToUpperCase(lhs.path().filename()) < Utils::FileNameToUpperCase(rhs.path().filename()));
			});
		}
		std::sort(m_DirectoryHierarchy.begin(), m_DirectoryHierarchy.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs.first.is_directory() && !rhs.first.is_directory() ||
				(((lhs.first.is_directory() && rhs.first.is_directory()) || (!lhs.first.is_directory() && !rhs.first.is_directory())) &&
				Utils::FileNameToUpperCase(lhs.first.path().filename()) < Utils::FileNameToUpperCase(rhs.first.path().filename()));
		});
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
		auto baseDirectoryIt = std::find_if(m_DirectoryHierarchy.begin(), m_DirectoryHierarchy.end(), [&baseDirectory](const auto& pair)
		{
			return pair.first == baseDirectory;
		});
		if (baseDirectoryIt == m_DirectoryHierarchy.cend()) return;

		for (const auto& it : baseDirectoryIt->second)
		{
			if (!it.is_directory()) continue;

			char directoryName[PATH_BUFFER_SIZE] = ICON_FA_FOLDER " ";
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
			DrawDirectoryNavigator();

			ImGui::Separator();

			if (!m_Filter.IsActive())
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
		}

		ImGui::EndChild();
	}

	void ContentBrowserPanel::DrawDirectoryNavigator()
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
		auto selectedPathIt = std::find_if(m_DirectoryHierarchy.begin(), m_DirectoryHierarchy.end(), [&](const auto& pair)
		{
			return pair.first == m_SelectedDirectory;
		});
		if (selectedPathIt == m_DirectoryHierarchy.cend()) return;

		bool bIsDirectoryEmpty = true;
		for (const auto& it : selectedPathIt->second)
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

	// TODO: Consider caching the result
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

	void ContentBrowserPanel::DrawWindowContextMenu()
	{
		// Right-click on blank space
		if (ImGui::BeginPopupContextWindowWithPadding(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			// TODO: Refactor: Actually create directory or asset after renaming

			if (ImGui::MenuItem(ICON_FA_FOLDER_PLUS "  Create New Folder"))
			{
				auto newPath = m_SelectedDirectory / "NewFolder";
				std::filesystem::create_directory(newPath);
				OnPathCreated(newPath);
			}

			ImGui::Separator();

			AssetManager::Get().ForEachAssetFactory([&](AssetTypeId typeId, const Ref<AssetFactoryBase>& factory)
			{
				char name[PATH_BUFFER_SIZE];
				strcpy_s(name, factory->GetAssetTypeIcon());
				strcat_s(name, " ");
				strcat_s(name, factory->GetAssetTypeName());
				if (ImGui::MenuItem(name))
				{
					char newName[PATH_BUFFER_SIZE] = "New";
					strcat_s(newName, factory->GetNormalizedAssetTypeName());
					strcat_s(newName, g_EngineAssetExtension);
					auto newPath = m_SelectedDirectory / newName;
					AssetManager::Get().CreateAsset(typeId, newPath.string());
					OnPathCreated(newPath);
				}
			});

			ImGui::EndPopup();
		}
	}

	void ContentBrowserPanel::DrawSelectablePath(const char* icon, const std::filesystem::path& path)
	{
		static constexpr float indentWidth = 5.0f;
		const bool bPathNeedsRenaming = m_PathJustCreated == path;
		ImGui::Indent(indentWidth);
		{
			char name[PATH_BUFFER_SIZE];
			strcpy_s(name, icon);
			if (!bPathNeedsRenaming)
			{
				strcat_s(name, path.stem().string().c_str());
			}
			else
			{
				ImGui::AlignTextToFramePadding();
			}
			if (ImGui::Selectable(name, m_SelectedPath == path))
			{
				m_SelectedPath = path;
			}
			if (bPathNeedsRenaming)
			{
				// Clear selection as long as renaming is in process
				m_SelectedPath.clear();

				ImGui::SameLine(0.0f, 0.0f);

				ImGui::SetKeyboardFocusHere();
				ImGui::SetNextItemWidth(-1.0f);
				static char renameBuffer[PATH_BUFFER_SIZE];
				strcpy_s(renameBuffer, path.stem().string().c_str());
				if (ImGui::InputText("##RenamePath", renameBuffer, sizeof(renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
				{
					// Add file extension .zasset automatically
					if (std::filesystem::directory_entry{ path }.is_regular_file())
					{
						strcat_s(renameBuffer, g_EngineAssetExtension);

					}
					const auto newPath = path.parent_path() / renameBuffer;
					std::filesystem::rename(path, newPath);
					OnPathRenamed(path, newPath);
					m_PathJustCreated.clear();
				}
			}
		}
		ImGui::Unindent(indentWidth);
	}

	void ContentBrowserPanel::DrawDirectory(const std::filesystem::path& path)
	{
		DrawSelectablePath(ICON_FA_FOLDER " ", path);
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
			AssetTypeId typeId = FileUtils::GetAssetTypeIdFromFile(path.string());
			m_AssetIcons[pathStr] = AssetManager::Get().GetAssetFactoryByAssetType(typeId)->GetAssetTypeIcon();
		}

		DrawSelectablePath(m_AssetIcons[pathStr], path);
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
		auto currentDirectory = directory;
		while (currentDirectory != m_AssetRootDirectory)
		{
			storage->SetInt(m_DirectorySpecs[currentDirectory.string()].TreeNodeId, true);
			currentDirectory = currentDirectory.parent_path();
		}
		storage->SetInt(m_DirectorySpecs[currentDirectory.string()].TreeNodeId, true);
	}

	void ContentBrowserPanel::HandleRightColumnAssetDoubleClicked(const std::filesystem::path& path)
	{
		AssetManager::Get().OpenAsset(path.string());
	}

	void ContentBrowserPanel::OnPathCreated(const std::filesystem::path& path)
	{
		m_PathJustCreated = path;

		auto& parentPath = path.parent_path();
		auto parentPathIt = std::find_if(m_DirectoryHierarchy.begin(), m_DirectoryHierarchy.end(), [&parentPath](const auto& pair)
		{
			return pair.first == parentPath;
		});
		ZE_CORE_ASSERT(parentPathIt != m_DirectoryHierarchy.end());

		parentPathIt->second.emplace_back(path);
		m_DirectoryHierarchy.emplace_back(std::make_pair(path, std::vector<std::filesystem::directory_entry>{}));

		SortDirectoryHierarchy();
	}

	void ContentBrowserPanel::OnPathRemoved(const std::filesystem::path& path)
	{
		auto& parentPath = path.parent_path();
		auto parentPathIt = std::find_if(m_DirectoryHierarchy.begin(), m_DirectoryHierarchy.end(), [&parentPath](const auto& pair)
		{
			return pair.first == parentPath;
		});
		ZE_CORE_ASSERT(parentPathIt != m_DirectoryHierarchy.end());

		auto currentPathInParentIt = std::find_if(parentPathIt->second.begin(), parentPathIt->second.end(), [&path](const auto& currentPath)
		{
			return currentPath == path;
		});
		ZE_CORE_ASSERT(currentPathInParentIt != parentPathIt->second.end());

		parentPathIt->second.erase(currentPathInParentIt);

		auto currentPathIt = std::find_if(m_DirectoryHierarchy.begin(), m_DirectoryHierarchy.end(), [&path](const auto& pair)
		{
			return pair.first == path;
		});
		ZE_CORE_ASSERT(currentPathIt != m_DirectoryHierarchy.end());

		m_DirectoryHierarchy.erase(currentPathIt);

		auto directoryIt = m_DirectorySpecs.find(path.string());
		if (directoryIt != m_DirectorySpecs.end())
		{
			m_DirectorySpecs.erase(directoryIt);
		}

		auto assetIt = m_AssetIcons.find(path.string());
		if (assetIt != m_AssetIcons.end())
		{
			m_AssetIcons.erase(assetIt);
		}
	}

	void ContentBrowserPanel::OnPathRenamed(const std::filesystem::path& oldPath, const std::filesystem::path& newPath)
	{
		m_SelectedPath = newPath;

		auto& parentPath = newPath.parent_path();
		auto currentPathIt = std::find_if(m_DirectoryHierarchy.begin(), m_DirectoryHierarchy.end(), [&oldPath](const auto& pair)
		{
			return pair.first == oldPath;
		});
		ZE_CORE_ASSERT(currentPathIt != m_DirectoryHierarchy.end());

		m_DirectoryHierarchy.emplace_back(std::make_pair(newPath, currentPathIt->second));
		m_DirectoryHierarchy.erase(currentPathIt);

		auto parentPathIt = std::find_if(m_DirectoryHierarchy.begin(), m_DirectoryHierarchy.end(), [&parentPath](const auto& pair)
		{
			return pair.first == parentPath;
		});
		ZE_CORE_ASSERT(parentPathIt != m_DirectoryHierarchy.end());
		auto currentPathInParentIt = std::find_if(parentPathIt->second.begin(), parentPathIt->second.end(), [&oldPath](const auto& currentPath)
		{
			return currentPath == oldPath;
		});
		ZE_CORE_ASSERT(currentPathInParentIt != parentPathIt->second.end());

		parentPathIt->second.erase(currentPathInParentIt);
		parentPathIt->second.emplace_back(newPath);

		auto directoryIt = m_DirectorySpecs.find(oldPath.string());
		if (directoryIt != m_DirectorySpecs.end())
		{
			m_DirectorySpecs.erase(directoryIt);
		}

		auto assetIt = m_AssetIcons.find(oldPath.string());
		if (assetIt != m_AssetIcons.end())
		{
			m_AssetIcons.erase(assetIt);
		}

		SortDirectoryHierarchy();
	}

}
