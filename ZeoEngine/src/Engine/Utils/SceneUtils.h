#pragma once

namespace ZeoEngine {

	class Scene;

	class SceneUtils
	{
	public:
		static bool IsLevelRuntime();
		static bool IsLevelPlaying();
		static Scene& GetActiveGameScene();
		static void OpenLevel(const std::string& path);
	};
	
}
