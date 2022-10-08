#pragma once

#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class Scene;

	class SceneUtils
	{
	public:
		static bool IsRuntime();
		static Ref<Scene> GetActiveGameScene();
		static void OpenLevel(const std::string& path);
	};
	
}
