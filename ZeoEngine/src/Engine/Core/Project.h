#pragma once

#include "Engine/Asset/Asset.h"

namespace ZeoEngine {

	struct ProjectConfig
	{
		std::string Name = "Untitled";
		std::string AssetDirectory;
		std::string ScriptAssemblyDirectory;

		AssetHandle DefaultLevelAsset = 0;
	};

	class Project
	{
	public:
		static Project* GetActive() { return s_ActiveProject.get(); }
		static auto& GetProjectLoadedDelegate() { return s_OnProjectLoaded; }
		static auto& GetProjectUnloadedDelegate() { return s_OnProjectUnloaded; }

		ProjectConfig& GetConfig() { return m_Config; }
		const ProjectConfig& GetConfig() const { return m_Config; }
		[[nodiscard]] std::string GetProjectDirectory() const;

		static Ref<Project> New();
		static Ref<Project> Load(const std::string& path);
		static void Unload();
		static void Save();

	private:
		std::string m_ProjectFilePath;
		ProjectConfig m_Config;

		inline static Ref<Project> s_ActiveProject;
		inline static entt::sigh<void()> s_OnProjectLoadedDel;
		inline static entt::sink<entt::sigh<void()>> s_OnProjectLoaded{ s_OnProjectLoadedDel };
		inline static entt::sigh<void()> s_OnProjectUnloadedDel;
		inline static entt::sink<entt::sigh<void()>> s_OnProjectUnloaded{ s_OnProjectUnloadedDel };
	};
	
}
