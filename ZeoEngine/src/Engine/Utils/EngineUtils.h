#pragma once

#include <magic_enum.hpp>

#include "Engine/Core/Application.h"
#include "Engine/Asset/Asset.h"

namespace ZeoEngine {
	
	class SceneRenderer;
	struct SceneContext;
	class RenderPass;

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

		static std::optional<int> StringToInt(const std::string& str)
		{
			try
			{
				return std::stoi(str);
			}
			catch (const std::exception&)
			{
				return {};
			}
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
		
		static U32 GetFieldSize(FieldType type)
		{
			switch (type)
			{
				case FieldType::Bool:	return sizeof(bool);
				case FieldType::I8:		return sizeof(I8);
				case FieldType::I16:	return sizeof(I16);
				case FieldType::I32:	return sizeof(I32);
				case FieldType::I64:	return sizeof(I64);
				case FieldType::U8:		return sizeof(U8);
				case FieldType::U16:	return sizeof(U16);
				case FieldType::U32:	return sizeof(U32);
				case FieldType::U64:	return sizeof(U64);
				case FieldType::Float:	return sizeof(float);
				case FieldType::Double:	return sizeof(double);
				case FieldType::Enum:	return 4;
				case FieldType::Vec2:	return sizeof(Vec2);
				case FieldType::Vec3:	return sizeof(Vec3);
				case FieldType::Vec4:	return sizeof(Vec4);
				case FieldType::Asset:	return sizeof(AssetHandle);
				case FieldType::Entity:	return sizeof(UUID);
			}
			ZE_CORE_ASSERT(false, "Unknown field type!");
			return 0;
		}

		template<typename Type>
		static Type GetDefaultMin()
		{
			Type min;
			if constexpr (std::is_same_v<Type, I8>)
			{
				min = INT8_MIN;
			}
			else if constexpr (std::is_same_v<Type, U8>) // Every else if also needs a "constexpr"
			{
				min = 0ui8;
			}
			else if constexpr (std::is_same_v<Type, I16>)
			{
				min = INT16_MIN;
			}
			else if constexpr (std::is_same_v<Type, U16>)
			{
				min = 0ui16;
			}
			else if constexpr (std::is_same_v<Type, I32>)
			{
				min = INT32_MIN;
			}
			else if constexpr (std::is_same_v<Type, U32>)
			{
				min = 0ui32;
			}
			else if constexpr (std::is_same_v<Type, I64>)
			{
				min = INT64_MIN;
			}
			else if constexpr (std::is_same_v<Type, U64>)
			{
				min = 0ui64;
			}
			else if constexpr (std::is_same_v<Type, float>)
			{
				min = -FLT_MAX;
			}
			else if constexpr (std::is_same_v<Type, double>)
			{
				min = -DBL_MAX;
			}
			return min;
		}

		template<typename Type>
		static Type GetDefaultMax()
		{
			Type max;
			if constexpr (std::is_same_v<Type, I8>)
			{
				max = INT8_MAX;
			}
			else if constexpr (std::is_same_v<Type, U8>)
			{
				max = UINT8_MAX;
			}
			else if constexpr (std::is_same_v<Type, I16>)
			{
				max = INT16_MAX;
			}
			else if constexpr (std::is_same_v<Type, U16>)
			{
				max = UINT16_MAX;
			}
			else if constexpr (std::is_same_v<Type, I32>)
			{
				max = INT32_MAX;
			}
			else if constexpr (std::is_same_v<Type, U32>)
			{
				max = UINT32_MAX;
			}
			else if constexpr (std::is_same_v<Type, I64>)
			{
				max = INT64_MAX;
			}
			else if constexpr (std::is_same_v<Type, U64>)
			{
				max = UINT64_MAX;
			}
			else if constexpr (std::is_same_v<Type, float>)
			{
				max = FLT_MAX;
			}
			else if constexpr (std::is_same_v<Type, double>)
			{
				max = DBL_MAX;
			}
			return max;
		}

		static SceneRenderer* GetSceneRendererFromContext(const SceneContext* sceneContext);

		template<typename T>
		static T* GetRenderPassFromContext(const SceneContext* sceneContext, const std::string& passName)
		{
			const auto sceneRenderer = GetSceneRendererFromContext(sceneContext);
			return sceneRenderer->GetRenderGraph().GetRenderPass<T>(passName);
		}

	};

}
