#pragma once

#include <unordered_map>

#include <imgui.h>

#include "Engine/Asset/Asset.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"
#include "Panels/PanelBase.h"

namespace ZeoEngine {

	class EditorPreviewWorldBase;
	class AssetPreviewWorldBase;
	class LevelPreviewWorld;
	class Scene;
	class SceneRenderer;
	class EditorMenu;
	class PanelBase;
	class FileWatcher;

	extern class Editor* g_Editor;

	class Editor
	{
		friend class EngineUtils;

	public:
		Editor();
		~Editor();

		void OnAttach();
		void OnUpdate(DeltaTime dt);
		void OnImGuiRender();
		void OnEvent(Event& e) const;

		const FileWatcher& GetFileWatcher() const { return *m_FileWatcher; }

		template<typename T, typename ... Args>
		T* CreateWorld(std::string worldName, Args&& ... args)
		{
			if (GetWorld(worldName))
			{
				ZE_CORE_ERROR("Failed to create {0}! World already exists!", worldName);
				return nullptr;
			}

			auto worldPtr = CreateScope<T>(worldName, std::forward<Args>(args)...);
			auto* world = worldPtr.get();
			m_Worlds.emplace(std::move(worldName), std::move(worldPtr));
			world->OnAttach();
			world->PostAttach();
			return world;
		}

		template<typename WorldClass = EditorPreviewWorldBase>
		WorldClass* GetWorld(const std::string& worldName) const
		{
			static_assert(std::is_base_of_v<EditorPreviewWorldBase, WorldClass>, "WorldClass must be derived from 'EditorPreviewWorldBase'!");

			const auto it = m_Worlds.find(worldName);
			if (it == m_Worlds.end()) return nullptr;

			return dynamic_cast<WorldClass*>(it->second.get());
		}

		template<typename T, typename ... Args>
		T* GetOrCreateWorld(const std::string& worldName, Args&& ... args)
		{
			auto* world = GetWorld<T>(worldName);
			if (!world)
			{
				return CreateWorld<T>(worldName, std::forward<Args>(args)...);
			}

			return world;
		}

		LevelPreviewWorld* GetLevelWorld() const;

		void NewLevel();
		void LoadLevel();
		void LoadLevel(const std::string& path) const;
		void SaveLevel() const;
		void SaveLevel(const std::string& path) const;
		void SaveLevelAs() const;

		bool ShouldRebuildDockspaceLayout() const { return m_bShouldRebuildDockspaceLayout; }
		void RebuildDockspaceLayout() { m_bShouldRebuildDockspaceLayout = true; }

		EditorMenu& CreateMenu(std::string menuName);

		template<typename T, typename ... Args>
		T* CreatePanel(std::string panelName, Args&& ... args)
		{
			if (GetPanel(panelName))
			{
				ZE_CORE_ERROR("Failed to create {0}! Panel already exists!", panelName);
				return {};
			}

			auto panelPtr = CreateScope<T>(panelName, std::forward<Args>(args)...);
			auto* panel = panelPtr.get();
			m_Panels.emplace(std::move(panelName), std::move(panelPtr));
			panel->OnAttach();
			panel->Toggle(true);
			return panel;
		}

		template<typename T, typename ... Args>
		T* OpenPanel(const std::string& panelName, Args&& ... args)
		{
			auto* panel = GetPanel<T>(panelName);
			if (!panel)
			{
				panel = CreatePanel<T>(panelName, std::forward<Args>(args)...);
			}

			panel->Toggle(true);
			return panel;
		}

		template<typename PanelClass = PanelBase>
		PanelClass* GetPanel(const std::string& panelName) const
		{
			static_assert(std::is_base_of_v<PanelBase, PanelClass>, "PanelClass must be derived from 'PanelBase'!");

			const auto it = m_Panels.find(panelName);
			if (it == m_Panels.end()) return nullptr;

			return dynamic_cast<PanelClass*>(it->second.get());
		}

		void InspectLevelEntity() const;
		void InspectAsset(const std::string& path, AssetPreviewWorldBase* world, bool bIsFromAssetBrowser, bool bFromHistory = false) const;
		void ClearInspect() const;

	private:
		void RenderDockspace();
		void BuildDockspaceLayout(ImGuiID dockspaceID);
		void BuildDockWindows(ImGuiID dockspaceID);
		void RenderMainMenuBar() const;

		void OnFileModified(const std::string& path) const;

	private:
		Scope<FileWatcher> m_FileWatcher;

		std::unordered_map<std::string, Scope<EditorPreviewWorldBase>> m_Worlds;

		std::vector<Scope<EditorMenu>> m_Menus;
		std::unordered_map<std::string, Scope<PanelBase>> m_Panels;

		bool m_bShouldRebuildDockspaceLayout = false;
	};

}
