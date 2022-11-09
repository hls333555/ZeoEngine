#pragma once

namespace ZeoEngine {

	class Scene;

	class SceneUtils
	{
	public:
		static bool IsLevelRuntime();
		static Scene& GetActiveGameScene();
		static void OpenLevel(const std::string& path);
	};
	
}
