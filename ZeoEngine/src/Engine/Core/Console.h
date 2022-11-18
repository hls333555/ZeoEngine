#pragma once

#include <optional>

namespace ZeoEngine {

	enum class CommandType
	{
		Default, // Command will appear in the filter list in all cases
		EditOnly, // Command will only appear in the filter list in edit mode
		RuntimeOnly // Command will only appear in the filter list in simulation or play mode
	};

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

		void RegisterVariable(std::string key, float defaultValue, std::string tooltip = "", CommandType type = CommandType::Default);
		void RegisterCommand(std::string key, CommandFuncType command, std::string tooltip = "", CommandType type = CommandType::Default);
		std::optional<float> GetVariableValue(const std::string& key);
		std::optional<float> GetVariableDefaultValue(const std::string& key);
		bool SetVariableValue(const std::string& key, std::optional<float> value);
		bool ExecuteCommand(const std::string& key, const std::vector<std::string>& params);
		std::string GetCommandTooltip(const std::string& key);
		bool IsConsoleVariable(const std::string& key);
		CommandType GetCommandType(const std::string& key);

		void ResetAllVariableValues();

	private:
		struct CommandData
		{
			CommandType Type;
			float DefaultValue = 0.0f;
			float CurrentValue = 0.0f;
			CommandFuncType CommandFunc;
			std::string Tooltip;

			bool IsVariable() const { return !CommandFunc;}
		};

		std::unordered_map<std::string, CommandData> m_Commands;
	};

}
