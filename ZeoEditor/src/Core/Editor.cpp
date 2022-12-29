#include "Core/Editor.h"

#include "Engine/Core/FileWatcher.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Engine/Core/Project.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/World.h"
#include "Engine/Profile/BenchmarkTimer.h"
#include "Engine/Scripting/ScriptEngine.h"
#include "Engine/Utils/PlatformUtils.h"
#include "Panels/LevelViewPanel.h"
#include "Panels/LevelOutlinePanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/StatsPanel.h"
#include "Panels/FrameDebugPanel.h"
#include "Panels/PreferencesPanel.h"
#include "Panels/AboutPanel.h"
#include "Menus/EditorMenu.h"
#include "Menus/EditorMenuItems.h"
#include "Panels/OpenAssetPanel.h"
#include "Worlds/LevelPreviewWorld.h"
#include "Worlds/AssetPreviewWorlds.h"

namespace ZeoEngine {

	Editor* g_Editor = nullptr;

	Editor::Editor()
	{
		g_Editor = this;
	}

	Editor::~Editor() = default;

	void Editor::OnAttach()
	{
		m_FileWatcher = CreateScope<FileWatcher>(std::chrono::duration<I32, std::milli>(1000));
		m_FileWatcher->m_OnFileModified.connect<&Editor::OnFileModified>(this);
		m_FileWatcher->m_OnFileModified.connect<&ScriptEngine::OnAssemblyChanged>();
		m_FileWatcher->m_OnFileAdded.connect<&ScriptEngine::OnAssemblyChanged>();

		const auto args = Application::Get().GetSpecification().CommandLineArgs;
		if (args.Count > 1)
		{
			OpenProject(args[1]);
		}
		else
		{
			if (!OpenProject())
			{
				Application::Get().Close();
				return;
			}
		}

		CreatePanel<LevelViewPanel>(LEVEL_VIEW);
		CreatePanel<LevelOutlinePanel>(LEVEL_OUTLINE);
		CreatePanel<InspectorPanel>(INSPECTOR);
		CreatePanel<ContentBrowserPanel>(CONTENT_BROWSER);
		CreatePanel<ConsolePanel>(CONSOLE);

		CreateMenu("File")
			.MenuItem<MenuItem_NewLevel>(ICON_FA_FILE "  New level", "CTRL+N")
			.MenuItem<MenuItem_LoadLevel>(ICON_FA_FILE_IMPORT "  Load level", "CTRL+O")
			.MenuItem<MenuItem_SaveLevel>(ICON_FA_SAVE "  Save level", "CTRL+S")
			.MenuItem<MenuItem_SaveLevelAs>(ICON_FA_SAVE "  Save level As", "CTRL+ALT+S")
			.MenuItem<MenuItem_Separator>()
			.MenuItem<MenuItem_OpenProject>(ICON_FA_FILE_IMPORT "  Open Project", "CTRL+ALT+O");

		CreateMenu("Edit")
			.MenuItem<MenuItem_Undo>(ICON_FA_UNDO "  Undo", "CTRL+Z")
			.MenuItem<MenuItem_Redo>(ICON_FA_REDO "  Redo", "CTRL+Y")
			.MenuItem<MenuItem_Copy>(ICON_FA_COPY"  Copy", "CTRL+C")
			.MenuItem<MenuItem_Paste>(ICON_FA_PASTE "  Paste", "CTRL+V")
			.MenuItem<MenuItem_Cut>(ICON_FA_CUT "  Cut", "CTRL+X");

		CreateMenu("Window")
			.MenuItem<MenuItem_TogglePanel<LevelViewPanel>>(LEVEL_VIEW)
			.MenuItem<MenuItem_TogglePanel<LevelOutlinePanel>>(LEVEL_OUTLINE)
			.MenuItem<MenuItem_TogglePanel<InspectorPanel>>(INSPECTOR)
			.MenuItem<MenuItem_TogglePanel<ContentBrowserPanel>>(CONTENT_BROWSER)
			.MenuItem<MenuItem_TogglePanel<ConsolePanel>>(CONSOLE)
			.MenuItem<MenuItem_TogglePanel<StatsPanel>>(STATS)
			.MenuItem<MenuItem_TogglePanel<FrameDebugPanel>>(FRAME_DEBUGGER)
			.MenuItem<MenuItem_TogglePanel<PreferencesPanel>>(PREFERENCES)
			.MenuItem<MenuItem_Separator>()
			.MenuItem<MenuItem_ResetLayout>(ICON_FA_WINDOW_RESTORE "  Reset layout");

		CreateMenu("Help")
			.MenuItem<MenuItem_TogglePanel<AboutPanel>>(ABOUT);

	}

	void Editor::OnDetach()
	{
		CloseProject();
	}

	void Editor::OnUpdate(DeltaTime dt)
	{
		for (const auto& [name, panel] : m_Panels)
		{
			panel->OnUpdate(dt);
		}

		for (const auto& [name, world] : m_Worlds)
		{
			world->OnUpdate(dt);
		}
	}

	void Editor::OnImGuiRender()
	{
		RenderMainMenuBar();
		RenderDockspace();
		for (const auto& [name, panel] : m_Panels)
		{
			panel->OnImGuiRender();
		}
		m_bShouldRebuildDockspaceLayout = false;
	}

	void Editor::OnEvent(Event& e) const
	{
		for (const auto& menu : m_Menus)
		{
			menu->OnEvent(e);
		}

		for (const auto& [name, panel] : m_Panels)
		{
			panel->OnEvent(e);
		}
	}

	LevelPreviewWorld* Editor::GetLevelWorld() const
	{
		return GetWorld<LevelPreviewWorld>("Level");
	}

	bool Editor::OpenProject()
	{
		const auto path = FileDialogs::Open(false, "ZeoEngine Project (*.zproject)\0*.zproject\0");
		if (path.empty()) return false;

		OpenProject(path[0]);
		return true;
	}

	void Editor::OpenProject(const std::string& path)
	{
		if (Project::GetActive())
		{
			CloseProject();
		}

		const auto project = Project::Load(path);
		ZE_CORE_ASSERT(project);

		if (!GetLevelWorld())
		{
			CreateWorld<LevelPreviewWorld>("Level");
		}

		const auto metadata = AssetRegistry::Get().GetAssetMetadata(Project::GetActive()->GetConfig().DefaultLevelAsset);
		if (metadata)
		{
			LoadLevel(metadata->Path);
		}
		else
		{
			NewLevel();
		}
	}

	void Editor::CloseProject()
	{
		Project::Unload();
	}

	void Editor::NewLevel() const
	{
		LoadLevel(Level::GetTemplatePath());
	}

	void Editor::LoadLevel()
	{
		if (auto* openAssetPanel = GetPanel<OpenAssetPanel>(OPEN_ASSET))
		{
			openAssetPanel->Toggle(true);
		}
		else
		{
			CreatePanel<OpenAssetPanel>(OPEN_ASSET, Level::TypeID());
		}
	}

	void Editor::LoadLevel(const std::string& path) const
	{
		auto* levelWorld = GetLevelWorld();
		ZE_CORE_ASSERT(levelWorld);

		Timer timer;
		// An empty scene is created every time, so we have to deserialize every time
		levelWorld->LoadAsset(path, true);
		ZE_CORE_WARN("Loading \"{0}\" took {1} ms", path, timer.ElapsedMillis());
	}

	void Editor::SaveLevel() const
	{
		const AssetHandle levelHandle = GetLevelWorld()->GetAsset()->GetHandle();
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(levelHandle);
		const auto& assetPath = metadata->Path;
		if (assetPath.empty() || metadata->IsTemplateAsset())
		{
			SaveLevelAs();
		}
		else
		{
			SaveLevel(assetPath);
		}
	}

	void Editor::SaveLevel(const std::string& path) const
	{
		GetLevelWorld()->SaveAsset(path);
	}

	void Editor::SaveLevelAs() const
	{
		GetLevelWorld()->SaveAssetAs();
	}

	EditorMenu& Editor::CreateMenu(std::string menuName)
	{
		Scope<EditorMenu> menu = CreateScope<EditorMenu>(std::move(menuName));
		EditorMenu& ret = *menu;
		m_Menus.emplace_back(std::move(menu));
		return ret;
	}

	void Editor::InspectLevelEntity() const
	{
		auto* inspectorPanel = GetPanel<InspectorPanel>(INSPECTOR);
		inspectorPanel->UpdateWorld(GetLevelWorld(), false);
		inspectorPanel->ToggleAssetView(false);
	}

	void Editor::InspectAsset(const std::string& path, AssetPreviewWorldBase* world, bool bIsFromAssetBrowser, bool bFromHistory) const
	{
		auto* inspectorPanel = GetPanel<InspectorPanel>(INSPECTOR);

		// If last world is level world, clear selected entity and re-add history data
		auto* inspectorWorld = inspectorPanel->GetEditorWorld();
		if (inspectorWorld == GetLevelWorld())
		{
			if (const Entity selectedEntity = inspectorWorld->GetContextEntity())
			{
				inspectorWorld->SetContextEntity({});
				inspectorPanel->ClearInspectHistory();
				inspectorPanel->AddInspectHistory({ inspectorWorld, selectedEntity });
			}
		}
		else if (!bFromHistory)
		{
			const auto metadata = AssetRegistry::Get().GetAssetMetadata(inspectorWorld->GetAsset()->GetHandle());
			inspectorPanel->AddInspectHistory({ inspectorWorld, metadata->Path });
		}

		Timer timer;
		world->LoadAsset(path, false);
		world->FocusContextEntity(true);
		ZE_CORE_WARN("Loading \"{0}\" took {1} ms", path, timer.ElapsedMillis());

		inspectorPanel->UpdateWorld(world, true);
		inspectorPanel->ToggleAssetView(world->GetAssetView());
		// If we inspect asset from the Content Browser Panel, the history will be cleared
		if (!bIsFromAssetBrowser && !bFromHistory)
		{
			inspectorPanel->ClearInspectHistory();
		}
	}

	void Editor::ClearInspect() const
	{
		const auto* inspectorPanel = GetPanel<InspectorPanel>(INSPECTOR);
		inspectorPanel->GetEditorWorld()->SetContextEntity({});
		inspectorPanel->ToggleAssetView(false);
	}

	void Editor::RenderDockspace()
	{
		const auto* viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags hostWindowFlags = 0;
		hostWindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
		hostWindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		hostWindowFlags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		char label[32];
		ImFormatString(label, IM_ARRAYSIZE(label), "DockSpaceViewport_%08X", viewport->ID);
		ImGui::Begin(label, nullptr, hostWindowFlags);
		ImGui::PopStyleVar(3);
		ImGuiID dockspaceID = ImGui::GetID("DockSpace");
		BuildDockspaceLayout(dockspaceID);
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::End();
	}

	void Editor::BuildDockspaceLayout(ImGuiID dockspaceID)
	{
		if (ImGui::DockBuilderGetNode(dockspaceID) == nullptr || m_bShouldRebuildDockspaceLayout)
		{
			// Clear out existing layout
			ImGui::DockBuilderRemoveNode(dockspaceID);
			// Add empty node
			ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
			// Main node should cover entire window
			ImGui::DockBuilderSetNodeSize(dockspaceID, ImGui::GetWindowSize());
			// Build layout
			BuildDockWindows(dockspaceID);
			ImGui::DockBuilderFinish(dockspaceID);
		}
	}

	void Editor::BuildDockWindows(ImGuiID dockspaceID)
	{
		ImGuiID dockLeft;
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Right, 0.2f, nullptr, &dockLeft);
		ImGuiID dockRightDown;
		ImGuiID dockRightUp = ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Up, 0.4f, nullptr, &dockRightDown);
		ImGuiID dockLeftUp;
		ImGuiID dockLeftDown = ImGui::DockBuilderSplitNode(dockLeft, ImGuiDir_Down, 0.3f, nullptr, &dockLeftUp);
		ImGuiID dockLeftUpRight;
		ImGuiID dockLeftUpLeft = ImGui::DockBuilderSplitNode(dockLeftUp, ImGuiDir_Left, 0.2f, nullptr, &dockLeftUpRight);
		ImGuiID dockLeftDownRight;
		ImGuiID dockLeftDownLeft = ImGui::DockBuilderSplitNode(dockLeftDown, ImGuiDir_Left, 0.5f, nullptr, &dockLeftDownRight);

		ImGui::DockBuilderDockWindow("###" LEVEL_VIEW, dockLeftUpRight);
		ImGui::DockBuilderDockWindow(LEVEL_OUTLINE, dockRightUp);
		ImGui::DockBuilderDockWindow(INSPECTOR, dockRightDown);
		ImGui::DockBuilderDockWindow(CONTENT_BROWSER, dockLeftDownLeft);
		ImGui::DockBuilderDockWindow(CONSOLE, dockLeftDownRight);
	}

	void Editor::RenderMainMenuBar() const
	{
		if (ImGui::BeginMainMenuBar())
		{
			const ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			for (const auto& menu : m_Menus)
			{
				menu->OnImGuiRender();
			}

			// Display engine stats at right corner of the main menu bar
			{
				#define STATS_TEXT "%.f FPS (%.2f ms)"
				const float statsWidth = ImGui::CalcTextSize(STATS_TEXT).x;
				ImGui::Indent(contentRegionAvailable.x - statsWidth);
				ImGui::Text(STATS_TEXT, ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
			}

			ImGui::EndMainMenuBar();
		}
	}

	void Editor::OnFileModified(const std::string& path) const
	{
		// Only process resource asset hot-reloading
		const std::string assetPath = path + AssetRegistry::GetAssetExtension();
		if (!AssetRegistry::Get().GetAssetMetadata(assetPath)) return;

		Application::Get().SubmitToMainThread([assetPath]()
		{
			// FileWatcher uses absolute path
			AssetLibrary::ReloadAsset(FileSystemUtils::GetStandardPath(assetPath));
		});
	}

}
