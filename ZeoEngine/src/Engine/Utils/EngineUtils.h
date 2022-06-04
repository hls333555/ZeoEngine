#pragma once

#include <magic_enum.hpp>

#include "Engine/Core/Application.h"

namespace ZeoEngine {
	
	class SceneRenderer;
	class Scene;

	class EngineUtils
	{
	public:
		// TODO:
		template<typename T>
		static const char* GetNameFromEnumType(T type)
		{
			return magic_enum::enum_name(type).data();
		}

		static float GetTimeInSeconds()
		{
			return Application::Get().GetTimeInSeconds();
		}

		static std::vector<std::string> SplitString(const std::string& str, char delimiter)
		{
			std::vector<std::string> tokens;
			std::string token;
			std::istringstream tokenStream(str);
			while (std::getline(tokenStream, token, delimiter))
			{
				tokens.emplace_back(std::move(token));
			}
			return tokens;
		}

		static Ref<SceneRenderer> GetSceneRendererFromContext(const Ref<Scene>& sceneContext);

	};

}
