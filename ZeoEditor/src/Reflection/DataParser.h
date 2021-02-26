#pragma once

#include <string>
#include <unordered_map>

#include "Engine/Core/ReflectionHelper.h"

namespace ZeoEngine {

	struct DataParser
	{
	public:
		bool ShouldHideData(entt::meta_data data, entt::meta_any& instance);

		void ClearCache();

	private:
		void ParseHideCondition(entt::meta_data data, entt::meta_any& instance, const std::string& hideConditionStr, const char* operatorToken, std::optional<bool>& shouldHide);

	private:
		struct HideConditionInfo
		{
			std::string KeyStr, ValueStr;
			bool bHasShownWarning = false;
		};
		/** Map from data id to HideConditionInfo */
		std::unordered_map<uint32_t, HideConditionInfo> m_HideConditionBuffers;
	};

}
