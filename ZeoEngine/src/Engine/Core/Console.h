#pragma once

#include <optional>

namespace ZeoEngine {

	class Console
	{
		using CommandFuncType = std::function<void(const std::vector<std::string>&)>;

	private:
		Console() = default;
		~Console() = default;
	public:
		Console(const Console&) = delete;
		Console& operator=(const Console&) = delete;

		static Console& Get()
		{
			static Console instance;
			return instance;
		}

		void RegisterVariable(std::string key, float defaultValue, std::string tooltip = "");
		void RegisterCommand(std::string key, CommandFuncType command, std::string tooltip = "");
		std::optional<float> GetVariableValue(const std::string& key);
		std::optional<float> GetVariableDefaultValue(const std::string& key);
		bool SetVariableValue(const std::string& key, std::optional<float> value);
		bool ExecuteCommand(const std::string& key, const std::vector<std::string>& params);
		std::string GetCommandTooltip(const std::string& key);
		bool IsConsoleVariable(const std::string& key);

	private:
		struct CommandData
		{
			float DefaultValue = 0.0f;
			float CurrentValue = 0.0f;
			CommandFuncType CommandFunc;
			std::string Tooltip;

			bool IsVariable() const { return !CommandFunc;}
		};

		std::unordered_map<std::string, CommandData> m_Commands;
	};

}
