#include "Inspectors/DataParser.h"

#include "Engine/Core/Log.h"
#include "Engine/Utils/ReflectionUtils.h"

namespace ZeoEngine {

	bool DataParser::ShouldHideData(entt::meta_data data, entt::meta_any& instance)
	{
		auto bIsHiddenInEditor = ReflectionUtils::DoesPropertyExist(Reflection::HiddenInEditor, data);
		if (bIsHiddenInEditor) return true;

		auto hideCondition = ReflectionUtils::GetPropertyValue<const char*>(Reflection::HideCondition, data);
		// HideCondition property is not set, show this data normally
		if (!hideCondition) return false;

		std::string hideConditionStr{ *hideCondition };
		std::optional<bool> shouldHide;
		// TODO: Add more operators
		ParseHideCondition(data, instance, hideConditionStr, "==", shouldHide);
		ParseHideCondition(data, instance, hideConditionStr, "!=", shouldHide);

		return *shouldHide;
	}

	void DataParser::ClearCache()
	{
		m_HideConditionBuffers.clear();
	}

	void DataParser::ParseHideCondition(entt::meta_data data, entt::meta_any& instance, const std::string& hideConditionStr, const char* operatorToken, std::optional<bool>& shouldHide)
	{
		// The string has been successfully parsed already, just return
		if (shouldHide) return;

		auto tokenPos = hideConditionStr.find(operatorToken);
		if (tokenPos == std::string::npos) return;

		std::string keyStr, valueStr;
		auto dataId = data.id();
		if (m_HideConditionBuffers.find(dataId) != m_HideConditionBuffers.end())
		{
			keyStr = m_HideConditionBuffers[dataId].KeyStr;
			valueStr = m_HideConditionBuffers[dataId].ValueStr;
		}
		else
		{
			keyStr = hideConditionStr.substr(0, tokenPos);
			// Erase tail blanks
			keyStr.erase(keyStr.find_last_not_of(" ") + 1);
			valueStr = hideConditionStr.substr(tokenPos + 2, hideConditionStr.size() - 1);
			// Erase head blanks
			valueStr.erase(0, valueStr.find_first_not_of(" "));
			// Extract enum value if necessary (e.g. SceneCamera::ProjectionType::Perspective -> Perspective)
			auto valuePos = valueStr.rfind("::");
			if (valuePos != std::string::npos)
			{
				valueStr.erase(0, valuePos + 2);
			}

			m_HideConditionBuffers[dataId].KeyStr = keyStr;
			m_HideConditionBuffers[dataId].ValueStr = valueStr;
		}

		auto keyData = entt::resolve(instance.type().info()).data(entt::hashed_string::value(keyStr.c_str()));
		if (!keyData)
		{
			if (!m_HideConditionBuffers[dataId].bHasShownWarning)
			{
				const char* dataName = ReflectionUtils::GetMetaObjectName(data);
				ZE_CORE_WARN("Invalid HideCondition key: '{0}' on data '{1}'!", keyStr, dataName);
				m_HideConditionBuffers[dataId].bHasShownWarning = true;
			}
			shouldHide = false;
			return;
		}

		auto keyDataValue = keyData.get(instance);

		// Bool
		{
			if (valueStr.find("true") != std::string::npos || valueStr.find("True") != std::string::npos)
			{
				auto boolValue = keyDataValue.cast<bool>();
				if (operatorToken == "==")
				{
					shouldHide = boolValue;
					return;
				}
				if (operatorToken == "!=")
				{
					shouldHide = !boolValue;
					return;
				}
			}
			if (valueStr.find("false") != std::string::npos || valueStr.find("False") != std::string::npos)
			{
				auto boolValue = keyDataValue.cast<bool>();
				if (operatorToken == "==")
				{
					shouldHide = !boolValue;
					return;
				}
				if (operatorToken == "!=")
				{
					shouldHide = boolValue;
					return;
				}
			}
		}

		// Enum
		{
			auto valueToCompare = keyData.type().data(entt::hashed_string::value(valueStr.c_str())).get({});
			if (operatorToken == "==")
			{
				shouldHide = keyDataValue == valueToCompare;
				return;
			}
			if (operatorToken == "!=")
			{
				shouldHide = keyDataValue != valueToCompare;
				return;
			}
		}
	}

}
