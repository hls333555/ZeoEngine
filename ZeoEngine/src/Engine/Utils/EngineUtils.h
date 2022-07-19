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

		static std::string GetCurrentTimeAndDate()
		{
		    const auto now = std::chrono::system_clock::now();
		    const auto time = std::chrono::system_clock::to_time_t(now);

		    std::stringstream ss;
			tm newTime;
			localtime_s(&newTime, &time);
		    ss << std::put_time(&newTime, "%Y.%m.%d-%H.%M.%S");
		    return ss.str();
		}

		// https://stackoverflow.com/a/16749483/13756224
		static std::vector<std::string> SplitString(const std::string& str, char delimiter, bool bFuzzyMatch = false)
		{
			std::vector<std::string> tokens;
			std::string token;
			std::istringstream tokenStream(str);
			while (std::getline(tokenStream, token, delimiter))
			{
				if (!bFuzzyMatch || token.length() > 0)
				{
					tokens.emplace_back(std::move(token));
				}
			}
			return tokens;
		}

		static std::optional<float> StringToFloat(const std::string& str)
		{
			try
			{
				return std::stof(str);
			}
			catch (const std::exception&)
			{
				return {};
			}
		}

		static Ref<SceneRenderer> GetSceneRendererFromContext(const Ref<Scene>& sceneContext);

	};

}
