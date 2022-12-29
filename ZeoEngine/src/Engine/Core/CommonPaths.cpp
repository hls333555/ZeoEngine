#include "ZEpch.h"
#include "Engine/Core/CommonPaths.h"

#include "Engine/Core/Project.h"

namespace ZeoEngine {

	std::string CommonPaths::GetProjectDirectory()
	{
		return Project::GetActive()->GetProjectDirectory();
	}

	std::string CommonPaths::GetProjectAssetDirectory()
	{
		return fmt::format("{}/{}", GetProjectDirectory(), Project::GetActive()->GetConfig().AssetDirectory);
	}

	std::string CommonPaths::GetAppAssemblyPath()
	{
		const auto& config = Project::GetActive()->GetConfig();
		return fmt::format("{}/{}/{}.dll", GetProjectAssetDirectory(), config.ScriptAssemblyDirectory, config.Name);
	}

	std::string CommonPaths::GetProjectSavedDirectory()
	{
		return fmt::format("{}/{}", GetProjectDirectory(), "Saved");
	}

	std::string CommonPaths::GetCPUProfileOutputDirectory()
	{
		return fmt::format("{}/{}", GetProjectSavedDirectory(), "Profiling/CPU");
	}

	std::string CommonPaths::GetGPUProfileOutputDirectory()
	{
		return fmt::format("{}/{}", GetProjectSavedDirectory(), "Profiling/GPU");
	}

	std::string CommonPaths::GetPhysXDebuggerOutputDirectory()
	{
		return fmt::format("{}/{}", GetProjectSavedDirectory(), "PhysXDebugInfo");
	}

}
