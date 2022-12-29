#pragma once

namespace ZeoEngine {

	class CommonPaths
	{
	public:
		static std::string GetProjectDirectory();
		static std::string GetProjectAssetDirectory();
		static std::string GetProjectAssetDirectoryStandard() { return "Game"; }
		static std::string GetEngineAssetDirectory() { return "assets"; }
		static std::string GetEngineAssetDirectoryStandard() { return "Engine"; }
		static std::string GetCoreAssemblyPath() { return "resources/scripts/ZeoEngine-ScriptCore.dll"; }
		static std::string GetAppAssemblyPath();

		static std::string GetProjectSavedDirectory();
		static std::string GetCPUProfileOutputDirectory();
		static std::string GetGPUProfileOutputDirectory();
		static std::string GetPhysXDebuggerOutputDirectory();
	};
	
}
