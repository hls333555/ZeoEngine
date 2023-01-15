#pragma once

#include <optional>

extern "C" {
	typedef struct _MonoClassField MonoClassField;
}

namespace ZeoEngine {

	struct ScriptField
	{
		std::string Name;
		FieldType Type;
		MonoClassField* ClassField = nullptr;

		bool HasAttribute(const std::string& name) const;

		template<typename T>
		std::optional<T> GetAttributeValue(const std::string& name) const
		{
			T value;
			const bool res = GetAttributeValueInternal(name, &value);
			if (!res) return {};
			return value;
		}

		template<>
		std::optional<std::string> GetAttributeValue(const std::string& name) const
		{
			std::string value;
			const bool res = GetAttributeValueInternal(name, value);
			if (!res) return {};
			return value;
		}

	private:
		bool GetAttributeValueInternal(const std::string& name, void* outValue) const;
		bool GetAttributeValueInternal(const std::string& name, std::string& outValue) const;
	};
	
}
