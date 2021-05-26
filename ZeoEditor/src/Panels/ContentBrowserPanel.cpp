#include "Panels/ContentBrowserPanel.h"

#include <IconsFontAwesome5.h>

#include "Engine/Debug/BenchmarkTimer.h"
#include "Engine/Core/Assert.h"
#include "Core/AssetManager.h"
#include "Engine/Utils/PathUtils.h"
#include "Utils/EditorUtils.h"
#include "Core/AssetManager.h"
#include "Core/AssetFactory.h"
#include "Core/AssetActions.h"
#include "Engine/Core/AssetRegistry.h"
#include "Engine/GameFramework/Scene.h"
#include "Engine/GameFramework/ParticleSystem.h"

#define MAX_PATH_SIZE 128

namespace ZeoEngine {

	namespace Utils {
	
		static const char* GetFormatedAssetTypeName(const char* typeName)
		{
			if (!typeName) return nullptr;

			char* newName = new char[strlen(typeName) + 1];
			strcpy_s(newName, strlen(typeName) + 1, typeName);
			int32_t i = 0, j = 0;
			for (; newName[i] != '\0'; ++i)
			{
				if (newName[i] != ' ')
				{
					newName[j++] = newName[i];
				}
				else if (newName[i] != '\0')
				{
					newName[i + 1] = std::toupper(newName[i + 1]);
				}
			}

			newName[j] = '\0';

			return newName;
		}

	}

	void ContentBrowserPanel::OnAttach()
	{
		m_SelectedDirectory = AssetRegistry::GetAssetRootDirectory();
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
					ProcessPathDeletion(m_SelectedPath);
				}
				break;
			// Rename selected directory or asset
			case Key::F2:
				if (!m_SelectedPath.empty())
				{
					m_PathToRename = m_SelectedPath;
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
			DrawDirectoryTree();
		}

		ImGui::EndChild();
	}

	void ContentBrowserPanel::DrawDirectoryTree()
	{
		m_LeftColumnWindowId = ImGui::GetCurrentWindow()->ID;

		ImGuiTreeNodeFlags flags = (m_SelectedDirectory == AssetRegistry::GetAssetRootDirectory() ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
		static const char* rootDirectoryName = ICON_FA_FOLDER " Assets";
		bool bIsRootTreeExpanded = ImGui::TreeNodeEx(rootDirectoryName, flags);
		AssetRegistry::Get().GetPathSpec<DirectorySpec>(AssetRegistry::GetAssetRootDirectory())->TreeNodeId = ImGui::GetCurrentWindow()->GetID(rootDirectoryName);
		if (ImGui::IsItemClicked())
		{
			m_SelectedDirectory = AssetRegistry::GetAssetRootDirectory();
		}
		if (bIsRootTreeExpanded)
		{
			//BenchmarkTimer bt;
			DrawDirectoryTreeRecursively(AssetRegistry::GetAssetRootDirectory());
			//std::cout << bt.ElapsedMillis() << std::endl;

			ImGui::TreePop();
		}
	}

	void ContentBrowserPanel::DrawDirectoryTreeRecursively(const std::string& baseDirectory)
	{
		AssetRegistry& ar = AssetRegistry::Get();
		ar.ForEachPathInDirectory(baseDirectory, [this, &ar](const std::string& path)
		{
			auto directorySpec = ar.GetPathSpec<DirectorySpec>(path);
			if (!directorySpec) return;

			char directoryName[MAX_PATH_SIZE] = ICON_FA_FOLDER " ";
			strcat_s(directoryName, directorySpec->PathName.c_str());
			ImGuiTreeNodeFlags flags = (m_SelectedDirectory == path ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
			if (!directorySpec->bHasAnySubDirectory)
			{
				flags |= ImGuiTreeNodeFlags_Leaf;
			}
			bool bIsTreeExpanded = ImGui::TreeNodeEx(directoryName, flags);
			directorySpec->TreeNodeId = ImGui::GetCurrentWindow()->GetID(directoryName);
			if (ImGui::IsItemClicked())
			{
				m_SelectedDirectory = path;
			}
			if (bIsTreeExpanded)
			{
				DrawDirectoryTreeRecursively(path);

				ImGui::TreePop();
			}
		});
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
		int32_t i = 0;
		PathUtils::ForEachParentPathInPath(m_SelectedDirectory, [this, &i](const std::string& name)
		{
			ImGui::SameLine(0.0f, 0.0f);
			if (ImGui::TransparentSmallButton(name.c_str()))
			{
				std::string jumpToDirectory;
				int32_t j = 0;
				PathUtils::ForEachParentPathInPath(m_SelectedDirectory, [&jumpToDirectory, i, &j](const std::string& name)
				{
					jumpToDirectory = PathUtils::AppendPath(jumpToDirectory, name);
					if (j++ == i) return true;
					return false;
				});
				m_SelectedDirectory = std::move(jumpToDirectory);
			}
			ImGui::SameLine(0.0f, 0.0f);
			ImGui::Text("/");

			++i;
			return false;
		});
	}

	void ContentBrowserPanel::DrawPathsInDirectory()
	{
		bool bIsDirectoryEmpty = true;
		AssetRegistry& ar = AssetRegistry::Get();
		ar.ForEachPathInDirectory(m_SelectedDirectory, [&](const std::string& path)
		{
			bIsDirectoryEmpty = false;
			auto assetSpec = ar.GetPathSpec<AssetSpec>(path);
			if (assetSpec)
			{
				DrawAsset(path);
			}
			else
			{
				DrawDirectory(path);
			}
		});

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
		//BenchmarkTimer bt;
		bool bIsFilteredEmpty = true;
		AssetRegistry::Get().ForEachPathInDirectoryRecursively(m_SelectedDirectory, [this, &bIsFilteredEmpty](const std::string& path)
		{
			auto assetSpec = AssetRegistry::Get().GetPathSpec<AssetSpec>(path);
			if (!assetSpec) return;

			if (m_Filter.PassFilter(assetSpec->PathName.c_str()))
			{
				bIsFilteredEmpty = false;
				DrawAsset(path);
			}
		});
		//std::cout << bt.ElapsedMillis() << std::endl;

		if (bIsFilteredEmpty)
		{
			m_Filter.DrawEmptyText();
		}
	}

	void ContentBrowserPanel::DrawWindowContextMenu()
	{
		// Right-click on blank space
		if (ImGui::BeginPopupContextWindowWithPadding(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			// Actual directory or asset is created after renaming

			if (ImGui::MenuItem(ICON_FA_FOLDER_PLUS "  Create New Folder"))
			{
				std::string newPath = GetAvailableNewPathName("NewFolder", false);
				RequestPathCreation(newPath, false);
			}

			ImGui::Separator();

			AssetManager::Get().ForEachAssetFactory([this](AssetTypeId typeId, const Ref<IAssetFactory>& factory)
			{
				if (!factory->ShouldShowInContextMenu()) return;

				char name[MAX_PATH_SIZE];
				strcpy_s(name, factory->GetAssetTypeIcon());
				strcat_s(name, " ");
				strcat_s(name, factory->GetAssetTypeName());
				if (ImGui::MenuItem(name))
				{
					char baseName[MAX_PATH_SIZE] = "New";
					strcat_s(baseName, Utils::GetFormatedAssetTypeName(factory->GetAssetTypeName()));
					std::string newPath = GetAvailableNewPathName(baseName, true);
					RequestPathCreation(newPath, true);
					AssetRegistry::Get().GetPathSpec<AssetSpec>(newPath)->TypeId = typeId;
				}
			});

			ImGui::EndPopup();
		}
	}

	std::string ContentBrowserPanel::GetAvailableNewPathName(const char* baseName, bool bIsAsset)
	{
		int32_t i = 0;
		char suffix[10];
		char newName[MAX_PATH_SIZE];
		strcpy_s(newName, baseName);
		if (bIsAsset)
		{
			strcat_s(newName, AssetRegistry::GetEngineAssetExtension());
		}
		std::string newPath = PathUtils::AppendPath(m_SelectedDirectory, newName);
		while (AssetRegistry::Get().ContainsPathInDirectory(m_SelectedDirectory, newPath))
		{
			_itoa_s(++i, suffix, 10);
			strcpy_s(newName, baseName);
			strcat_s(newName, suffix);
			if (bIsAsset)
			{
				strcat_s(newName, AssetRegistry::GetEngineAssetExtension());
			}
			newPath = PathUtils::AppendPath(m_SelectedDirectory, newName);
		}
		return newPath;
	}

	void ContentBrowserPanel::DrawSelectablePath(const char* icon, const std::string& path)
	{
		static constexpr float indentWidth = 5.0f;
		const bool bPathNeedsRenaming = m_PathToRename == path;
		ImGui::Indent(indentWidth);
		{
			auto spec = AssetRegistry::Get().GetPathSpec(path);
			auto pathName = spec->PathName.c_str();
			char name[MAX_PATH_SIZE];
			strcpy_s(name, icon);
			if (!bPathNeedsRenaming)
			{
				strcat_s(name, " ");
				strcat_s(name, pathName);
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

				static bool bHasKeyboardFocused = false;
				if (!bHasKeyboardFocused)
				{
					// Focus input once
					ImGui::SetKeyboardFocusHere();
					bHasKeyboardFocused = true;
				}
				ImGui::SetNextItemWidth(-1.0f);
				char renameBuffer[MAX_PATH_SIZE];
				strcpy_s(renameBuffer, pathName);
				ImGui::InputText("##RenamePath", renameBuffer, sizeof(renameBuffer), ImGuiInputTextFlags_AutoSelectAll);
				// TODO: The following will not get called on right-click
				if (ImGui::IsItemDeactivated())
				{
					auto assetSpec = std::dynamic_pointer_cast<AssetSpec>(spec);
					if (assetSpec)
					{
						// Add engine file extension automatically
						strcat_s(renameBuffer, AssetRegistry::GetEngineAssetExtension());
					}
					std::string newPath = PathUtils::AppendPath(m_SelectedDirectory, renameBuffer);
					if (newPath != path && AssetRegistry::Get().ContainsPathInDirectory(m_SelectedDirectory, newPath))
					{
						ZE_CORE_WARN("Failed to rename \"{0}\" to \"{1}\"! Path already exist.", path, newPath);
						newPath = path;
					}
					ProcessPathRenaming(path, newPath, assetSpec);
					bHasKeyboardFocused = false;
				}
			}
		}
		ImGui::Unindent(indentWidth);
	}

	void ContentBrowserPanel::DrawDirectory(const std::string& path)
	{
		DrawSelectablePath(ICON_FA_FOLDER " ", path);
		// Double-click to open directory in the right column
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
		{
			m_SelectedDirectory = path;
			HandleRightColumnDirectoryDoubleClicked(path);
		}
	}

	void ContentBrowserPanel::DrawAsset(const std::string& path)
	{
		AssetTypeId typeId = AssetRegistry::Get().GetPathSpec<AssetSpec>(path)->TypeId;
		const char* icon = AssetManager::Get().GetAssetFactoryByAssetType(typeId)->GetAssetTypeIcon();
		DrawSelectablePath(icon, path);
		// Double-click to open asset editor
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
		{
			HandleRightColumnAssetDoubleClicked(path);
		}
	}

	void ContentBrowserPanel::HandleRightColumnDirectoryDoubleClicked(const std::string& directory)
	{
		ImGuiStorage* storage = ImGui::FindWindowByID(m_LeftColumnWindowId)->DC.StateStorage;
		// Manually toggle upper-level tree node open iteratively
		auto currentDirectory = directory;
		auto& ar = AssetRegistry::Get();
		while (currentDirectory != AssetRegistry::GetAssetRootDirectory())
		{
			storage->SetInt(ar.GetPathSpec<DirectorySpec>(currentDirectory)->TreeNodeId, true);
			currentDirectory = PathUtils::GetParentPath(currentDirectory);
		}
		storage->SetInt(ar.GetPathSpec<DirectorySpec>(currentDirectory)->TreeNodeId, true);
	}

	void ContentBrowserPanel::HandleRightColumnAssetDoubleClicked(const std::string& path)
	{
		AssetManager::Get().OpenAsset(path);
	}

	void ContentBrowserPanel::RequestPathCreation(const std::string& path, bool bIsAsset)
	{
		m_PathToRename = path;
		m_PathToCreate = path;
		AssetRegistry::Get().OnPathCreated(path, bIsAsset);
	}

	void ContentBrowserPanel::ProcessPathDeletion(const std::string& path)
	{
		// TODO: Pop up a deletion dialog
		PathUtils::DeletePath(path);
		AssetRegistry::Get().OnPathRemoved(path);
		m_SelectedPath.clear();
	}

	void ContentBrowserPanel::ProcessPathRenaming(const std::string& oldPath, const std::string& newPath, const Ref<AssetSpec>& assetSpec)
	{
		if (m_PathToCreate.empty())
		{
			// Regular renaming process
			PathUtils::RenamePath(oldPath, newPath);
		}
		else
		{
			// Triggered by path creation
			if (assetSpec)
			{
				AssetManager::Get().CreateAsset(assetSpec->TypeId, newPath);
			}
			else
			{
				PathUtils::CreateDirectory(newPath);
			}
			m_PathToCreate.clear();
		}
		m_SelectedPath = newPath;
		if (newPath != oldPath)
		{
			AssetRegistry::Get().OnPathRenamed(oldPath, newPath, static_cast<bool>(assetSpec));
		}
		m_PathToRename.clear();
	}

}
